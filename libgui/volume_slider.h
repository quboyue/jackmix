

#ifndef VOLUME_SLIDER_H
#define VOLUME_SLIDER_H

#include "abstractslider.h"
#include <dbvolcalc.h>
namespace JackMix {
    namespace GUI {

        class volume_slider : public QWidget,public dB2VolCalc
        {
            Q_OBJECT
        public:
            
            double _value;
            volume_slider(double value, double min, double max, int precision, double pagestep, QWidget*, QString = "%1 dB");
            ~volume_slider();

            virtual void value(double n, bool show_numeric = true);

        protected:
            void paintEvent(QPaintEvent*);

            void mouseEvent(QMouseEvent*);
            QRect _faderarea;
        public slots:
            void receive_OutputVolume(QString which, float max);
        private slots:
            void timeout();



        };

    }; // GUI
}; //JackMix

#endif // VOLUME_SLIDER_H

