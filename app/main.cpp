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

typedef struct
{
	MonoString* Name;
	intptr_t Handle;
} OperationData;

class MonoEmbedHelper
{
public:
	MonoEmbedHelper()
	{
		QString path = QFileInfo( QCoreApplication::applicationFilePath() ).absolutePath();
	#ifdef WIN32
		path += "/../";
	#else
		path += "/../../../";
	#endif
		std::string file (path.toUtf8().constData());
		mono_jit_init (file.c_str());

		QString corePath(path);
		corePath += "Core.dll";
		LoadCore(corePath);
		ProcessAssemblies(path);
	}

	const op_vector& GetOperations()
	{
		return _operations;
	}

	double ExecuteOperation(int gchandle, double a, double b)
	{
		void* args[2];
		args[0] = &a;
		args[1] = &b;
		MonoObject* object = mono_gchandle_get_target (gchandle);
		MonoMethod* virtualExecute = mono_object_get_virtual_method (object, _executeMethod);
		MonoObject* boxedResult = mono_runtime_invoke(virtualExecute, object, args, NULL);
		double d = *(double*)mono_object_unbox (boxedResult);

		return d;
	}

private:

	void ProcessAssemblies(const QString& path)
	{
		_embedHelperClass = mono_class_from_name (_coreImage, "EmbedSample", "EmbedHelper");
		_getOperationsMethod = mono_class_get_method_from_name (_embedHelperClass, "GetOperations", 1);

		void* args[1];
		args[0] = mono_string_new (mono_domain_get (), path.toUtf8().constData());
		MonoArray* operationsData = (MonoArray*)mono_runtime_invoke (_getOperationsMethod, NULL, args, NULL);

		for (int i = 0; i < mono_array_length (operationsData); i++)
		{
			OperationData operationData = mono_array_get (operationsData, OperationData, i);
			_operations.push_back(op_pair((QChar*)mono_string_chars(operationData.Name), operationData.Handle));
		}
	}

	void LoadCore(QString path)
	{
		std::string file (path.toUtf8().constData());
		MonoAssembly *assembly = mono_domain_assembly_open (mono_domain_get(), file.c_str());
		_coreImage = mono_assembly_get_image (assembly);

		_operationItfClass = mono_class_from_name (_coreImage, "EmbedSample", "IOperation");
		_executeMethod = mono_class_get_method_from_name (_operationItfClass, "Execute", 2);
	}

	std::vector<op_pair> _operations;
	MonoImage* _coreImage;
	MonoClass* _operationItfClass;
	MonoMethod* _executeMethod;
	MonoClass* _embedHelperClass;
	MonoMethod* _getOperationsMethod;
};

class MyDropDown : public QComboBox
{
public:
	MyDropDown(MonoEmbedHelper* embedHelper) : 
		_embedHelper(embedHelper)
	{
		for (int i = 0; i < _embedHelper->GetOperations().size(); i++)
		{
			op_pair operation = _embedHelper->GetOperations().at(i);
			this->addItem(operation.first, operation.second);
		}
	}

private:
	MonoEmbedHelper* _embedHelper;
};

class ExecuteButton : public QPushButton
{
  public:
    ExecuteButton(MonoEmbedHelper* embedHelper, MyDropDown* dropDown, QLineEdit* lineEdit1, QLineEdit* lineEdit2, QLineEdit* lineEdit3, QWidget *parent = 0) : 
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
	MonoEmbedHelper* _embedHelper;
	MyDropDown* _dropDown;
	QLineEdit* _lineEdit1;
	QLineEdit* _lineEdit2;
	QLineEdit* _lineEdit3;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	MonoEmbedHelper embedHelper;

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
