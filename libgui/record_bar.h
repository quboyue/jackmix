

#ifndef RECORD_BAR_H
#define RECORD_BAR_H
#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QTime>
#include <QTimer>
namespace JackMix {
    namespace GUI {


        class record_bar : public QWidget
        {
            Q_OBJECT
        public:
            QHBoxLayout* layout;
            record_bar();
            ~record_bar();
            QPushButton* record_button;
            QLabel* time_lable;
            QLabel* file_name_lable;
            QTimer* timer =new QTimer(this);
            QDateTime start_time;
        public slots:
           void is_record(bool toggled);
            void updateTime();

        };




    }; // GUI
}; //JackMix

#endif // RECORD_BAR_H

