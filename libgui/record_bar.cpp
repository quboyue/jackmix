

#include "record_bar.h"
#include "record_bar.moc"
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtGui/QPainter>
#include <QtWidgets/QStyleOption>
#include <QtWidgets/QStyle>
#include <QtGui/QMouseEvent>
#include <QtCore/QDebug>
#include <QTime>
#include <QTimer>
using namespace JackMix;
using namespace JackMix::GUI;


 record_bar::record_bar(){


	 connect(timer, SIGNAL(timeout()),this, SLOT(updateTime()));


	 layout = new  QHBoxLayout(this);
	 layout->setMargin(2);
	 layout->setSpacing(2);

	 record_button = new QPushButton("Record");
	 layout->addWidget(record_button,0);
	 record_button->setCheckable(true);
	 record_button->setStyleSheet("background-color: rgb(255,150,150)");
	 connect(record_button, SIGNAL(toggled(bool)), this, SLOT(is_record(bool)));



		 time_lable = new QLabel();
		 time_lable->setText("00:00:00");
		 layout->addWidget(time_lable, 2);

		 file_name_lable = new QLabel(" ");
		 layout->addWidget(file_name_lable, 3);



	 qDebug() << "  THIS IS record_bar BAR";




}

 record_bar::~record_bar() {}

 void record_bar::is_record(bool toggled) {
 
	 if (toggled) {
		 record_button->setText("Recording");
		 QString file_name = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm") + "-channel-*"  + ".wav";
		 file_name_lable->setText(file_name);
		 time_lable->setText("00:00:00");
		 timer->start(1000);
		 start_time = QDateTime::currentDateTime();
	 }
	 else 
	 {
		 timer->stop();
		 record_button->setText("Record");
		 file_name_lable->setText(" ");
		 time_lable->setText("00:00:00");
		
	 }
 
 
 } 
	 
 
 void record_bar::updateTime()  {
 


	 QTime show_time;
	 show_time.setHMS(0, 0, 0, 0);
	 time_lable->setText(show_time.addSecs(start_time.secsTo(QDateTime::currentDateTime())).toString("hh:mm:ss"));
 
 }