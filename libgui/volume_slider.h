

#ifndef VOLUME_SLIDER_H
#define VOLUME_SLIDER_H

#include "abstractslider.h"

namespace JackMix {
    namespace GUI {

        class volume_slider : public AbstractSlider
        {
            Q_OBJECT
        public:
            volume_slider(double value, double min, double max, int precision, double pagestep, QWidget*, QString = "%1 dB");
            ~volume_slider();

            virtual void value(double n, bool show_numeric = true);

        protected:
            void paintEvent(QPaintEvent*);

            void mouseEvent(QMouseEvent*);
            QRect _faderarea;

        private slots:
            void timeout();

        private:
            QTimer* _timer;
            bool _show_value;
        };

    }; // GUI
}; //JackMix

#endif // VOLUME_SLIDER_H

