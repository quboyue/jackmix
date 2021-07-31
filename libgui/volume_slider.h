

#ifndef VOLUME_SLIDER_H
#define VOLUME_SLIDER_H

#include "abstractslider.h"
#include <dbvolcalc.h>
#include <QtWidgets/QLayout>
namespace JackMix {
    namespace GUI {

        class volume_slider;
        class volume_bar : public QWidget, public dB2VolCalc
        {
            Q_OBJECT
        public:
            QHBoxLayout* layout;
            volume_bar();
            ~volume_bar();
            QList <volume_slider*> _volume_sliders;
            QStringList _outchannels;
        public slots:
            void receive_OutputVolume(QString which, float max);
        };


        class volume_slider : public QWidget,public dB2VolCalc
        {
            Q_OBJECT
        public:
            QString _name;
            double _value;
            volume_slider(double value, double min, double max, int precision, double pagestep, QWidget*,QString name);
            ~volume_slider();

        protected:
            void paintEvent(QPaintEvent*);
            QRect _faderarea;





        };

    }; // GUI
}; //JackMix

#endif // VOLUME_SLIDER_H

