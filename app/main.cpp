#include <QApplication>
#include <QFont>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QComboBox>

#include <QFileInfo>
#include <QDir>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <string>
#include <vector>

typedef std::pair<QString, int> op_pair;
typedef std::vector<op_pair> op_vector;

#ifdef WIN32
#include "MSCorEE.h"
#endif

typedef struct
{
	QChar* Name;
	intptr_t Handle;
} OperationData;

typedef struct
{
	int Length;
	void* Data;
} ArrayData;

#ifdef WIN32
#define CALLING_CONV __stdcall
#else
#define CALLING_CONV
#endif

typedef ArrayData (CALLING_CONV *GetOperationsFunc)(const ushort * path);
typedef double (CALLING_CONV *ExecuteFunc)(intptr_t handle, double a, double b);

typedef struct 
{
	GetOperationsFunc GetOperations;
	ExecuteFunc Execute;
} EmbedAPI;

class DotNetEmbedHelper
{
public:
	static EmbedAPI* GetEmbedAPI(const QString& path)
	{
		ICLRRuntimeHost* pHost;
		HRESULT hr = CorBindToRuntimeEx (NULL, NULL, 0, CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, (void**)&pHost);
		DWORD ret = 0;
		QString assPath = path + "Core.dll";
		
		EmbedAPI* api = (EmbedAPI*)malloc(sizeof(EmbedAPI));
		memset(api, 0, sizeof(EmbedAPI));
		QString arg = QString::number((qlonglong)&api);
		hr = pHost->Start();
		hr = pHost->ExecuteInDefaultAppDomain(assPath.utf16(), L"EmbedSample.EmbedHelper", L"DotNetEntryPoint", arg.utf16(), &ret);

		return api;
	}
};

class MonoEmbedHelper
{
public:
	static EmbedAPI* GetEmbedAPI(const QString& path)
	{
		std::string file (path.toUtf8().constData());
		mono_jit_init (file.c_str());

		QString corePath(path);
		corePath += "Core.dll";
		return LoadCore(corePath);
	}

private:
	static EmbedAPI* LoadCore(QString path)
	{
		std::string file (path.toUtf8().constData());
		MonoAssembly *assembly = mono_domain_assembly_open (mono_domain_get(), file.c_str());
		_coreImage = mono_assembly_get_image (assembly);

		MonoClass* embedHelperClass = mono_class_from_name (_coreImage, "EmbedSample", "EmbedHelper");
		MonoMethod* getAPIMethod = mono_class_get_method_from_name (embedHelperClass, "GetAPI", 0);
		MonoObject* boxedEmbedAPI = mono_runtime_invoke (getAPIMethod, NULL, NULL, NULL);
		return  *(EmbedAPI**)mono_object_unbox (boxedEmbedAPI);
	}

	static MonoImage* _coreImage;
};

MonoImage* MonoEmbedHelper::_coreImage = 0;

class CLREmbedHelper
{
public:
	CLREmbedHelper()
	{
		QString path = QFileInfo( QCoreApplication::applicationFilePath() ).absolutePath();
	#ifdef WIN32
		path += "/../";
	#else
		path += "/../../../";
	#endif
		std::string file (path.toUtf8().constData());
#if 0
		_embedAPI = MonoEmbedHelper::GetEmbedAPI(path);
#else
		_embedAPI = DotNetEmbedHelper::GetEmbedAPI(path);
#endif
		ProcessAssemblies(path);
	}

	const op_vector& GetOperations()
	{
		return _operations;
	}

	double ExecuteOperation(int gchandle, double a, double b)
	{
		double d = _embedAPI->Execute(gchandle, a, b);

		return d;
	}

private:

	void ProcessAssemblies(const QString& path)
	{
		ArrayData arrayData = _embedAPI->GetOperations(path.utf16());

		for (int i = 0; i < arrayData.Length; i++)
		{
			OperationData* operationData = (OperationData*)((char*)arrayData.Data + i*sizeof(OperationData));
			_operations.push_back(op_pair(QString(operationData->Name), operationData->Handle));
			MarshalFree(operationData->Name);
		}
		
		MarshalFree(arrayData.Data);
	}

	void MarshalFree(void* ptr)
	{
#ifdef WIN32
		GlobalFree(ptr);
#else
		// should use g_free
		free(ptr);
#endif
	}

	std::vector<op_pair> _operations;
	EmbedAPI* _embedAPI;
};

class MyDropDown : public QComboBox
{
public:
	MyDropDown(CLREmbedHelper* embedHelper) : 
		_embedHelper(embedHelper)
	{
		for (int i = 0; i < _embedHelper->GetOperations().size(); i++)
		{
			op_pair operation = _embedHelper->GetOperations().at(i);
			this->addItem(operation.first, operation.second);
		}
	}

private:
	CLREmbedHelper* _embedHelper;
};

class ExecuteButton : public QPushButton
{
  public:
    ExecuteButton(CLREmbedHelper* embedHelper, MyDropDown* dropDown, QLineEdit* lineEdit1, QLineEdit* lineEdit2, QLineEdit* lineEdit3, QWidget *parent = 0) : 
    	QPushButton(parent), 
		_embedHelper(embedHelper),
		_dropDown(dropDown),
    	_lineEdit1(lineEdit1),
    	_lineEdit2(lineEdit2),
    	_lineEdit3(lineEdit3)
    {
    	this->setText("Execute");
    }

  protected:
  	void mouseReleaseEvent ( QMouseEvent * event )
	{
		if(event->button() == Qt::LeftButton)
		{
			QString addend1 = _lineEdit1->text();
			QString addend2 = _lineEdit2->text();
			double d1 = addend1.toDouble();
			double d2 = addend2.toDouble();

			int index = _dropDown->currentIndex();
			int gchandle = _dropDown->itemData(index).toInt();
			double d = _embedHelper->ExecuteOperation(gchandle, d1, d2);

			QString sum = QString::number(d);
			_lineEdit3->setText(sum);

		} 
		QPushButton::mouseReleaseEvent(event);
	} 

private:
	CLREmbedHelper* _embedHelper;
	MyDropDown* _dropDown;
	QLineEdit* _lineEdit1;
	QLineEdit* _lineEdit2;
	QLineEdit* _lineEdit3;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	CLREmbedHelper embedHelper;

    QWidget window;
    window.resize(200, 120);

    QLineEdit lineEdit;
    QLineEdit lineEdit2;
    QLineEdit lineEdit3;
    lineEdit3.setReadOnly(true);
	
	MyDropDown dropDown(&embedHelper);

    ExecuteButton executeButton(&embedHelper, &dropDown, &lineEdit, &lineEdit2, &lineEdit3);
    executeButton.setFont(QFont("Times", 18, QFont::Bold));
	executeButton.setGeometry(10, 40, 180, 40);


    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(&lineEdit);
    layout->addWidget(&lineEdit2);
    layout->addWidget(&dropDown);
    layout->addWidget(&executeButton);
    layout->addWidget(&lineEdit3);
    window.setLayout(layout);


    window.show();
    return app.exec();
}
