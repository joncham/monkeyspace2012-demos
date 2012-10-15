#include <QApplication>
#include <QFont>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>
#include <QVBoxLayout>
#include <QMouseEvent>

#include <QFileInfo>
#include <QDir>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

static MonoMethod* s_Add;

class Adder : public QPushButton
{
  public:
    Adder(QLineEdit* lineEdit1, QLineEdit* lineEdit2, QLineEdit* lineEdit3, QWidget *parent = 0) : 
    	QPushButton(parent), 
    	_lineEdit1(lineEdit1),
    	_lineEdit2(lineEdit2),
    	_lineEdit3(lineEdit3)
    {
    	this->setText("Add");
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

			void* args[2];
			args[0] = &d1;
			args[1] = &d2;
			MonoObject* boxedResult = mono_runtime_invoke(s_Add, NULL, args, NULL);
			double d = *(double*)mono_object_unbox (boxedResult);

			QString sum = QString::number(d);
			_lineEdit3->setText(sum);

		} 
		QPushButton::mouseReleaseEvent(event);
	} 

private:
	QLineEdit* _lineEdit1;
	QLineEdit* _lineEdit2;
	QLineEdit* _lineEdit3;
};

void init_mono()
{
	QString path = QFileInfo( QCoreApplication::applicationFilePath() ).absolutePath();
#ifdef WIN32
	path += "/../Add.dll";
#else
	std::string file("/Users/jonathan/Development/monkeyspace2012-demos/app/Add.dll");
#endif
	std::string file (path.toUtf8().constData());
	MonoDomain* domain = mono_jit_init (file.c_str());
	MonoAssembly *assembly = mono_domain_assembly_open (domain, file.c_str());
	MonoImage* image = mono_assembly_get_image (assembly);

	MonoClass* klass = mono_class_from_name (image, "EmbedSample", "Adder");
	s_Add = mono_class_get_method_from_name (klass, "Add", 2);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    init_mono ();

    QWidget window;
    window.resize(200, 120);

    QLineEdit lineEdit;
    QLineEdit lineEdit2;
    QLineEdit lineEdit3;
    lineEdit3.setReadOnly(true);

    Adder add(&lineEdit, &lineEdit2, &lineEdit3);
    add.setFont(QFont("Times", 18, QFont::Bold));
	add.setGeometry(10, 40, 180, 40);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(&lineEdit);
    layout->addWidget(&lineEdit2);
    layout->addWidget(&add);
    layout->addWidget(&lineEdit3);
    window.setLayout(layout);


    window.show();
    return app.exec();
}
