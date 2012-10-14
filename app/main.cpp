#include <QApplication>
#include <QFont>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>
#include <QVBoxLayout>
#include <QMouseEvent>

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
			QString sum = QString::number(d1+d2);
			_lineEdit3->setText(sum);

		} 
		QPushButton::mouseReleaseEvent(event);  
	} 

private:
	QLineEdit* _lineEdit1;
	QLineEdit* _lineEdit2;
	QLineEdit* _lineEdit3;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

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