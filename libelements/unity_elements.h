/*
This is a demo used to add a new kind of elements which used to control unity gain and loss

*/

#ifndef UNITY_ELEMENTS_H
#define UNITY_ELEMENTS_H


#include <QtCore/QVector>

#include <mixingmatrix.h>
#include <dbvolcalc.h>
#include <controlreceiver.h>

namespace JackMix {

    namespace GUI {
        class Knob;
        class MidiControlChannelAssigner;
    }

    namespace MixerElements {

        class UnityElement : public JackMix::MixingMatrix::Element
            , public dB2VolCalc {
            Q_OBJECT
        public:

            UnityElement(QStringList inchannel, QStringList outchannel, MixingMatrix::Widget*, const char* = 0);
            ~UnityElement();

            int inchannels() const { return 1; }
            int outchannels() const { return 1; }

            void setIndicator(const QColor& c);

            // wether mute button was pushed
            bool is_mute = false;

            //value showed by knob's indicator 
            double indicator_value = -42;
        signals:
            void sendKnobPointer_signal(double);
            void sendUnityMute_signal(bool);
        public slots:
            void emitvalue(double);
            //delete me!!!
            void slot_mute_channel(bool);
            //delete me!!!
            void mouseMoveEvent(QMouseEvent* event);
            void mousePressEvent(QMouseEvent* event);
        private:
     
            JackMix::GUI::Knob* _poti;
            //delete me!!!
            QPushButton* MuteButton;
            //delete me!!!
        };

        void init_unity_elements();

    }; 
}; 

#endif 

