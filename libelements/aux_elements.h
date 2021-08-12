/*
    Copyright 2004 - 2007 Arnold Krille <arnold@arnoldarts.de>

    Modified to permit control via MIDI by Nick Bailey <nick@n-ism.org>
    Released as version 0.5.0 (et seq.) copyright 2013.
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation;
    version 2 of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef AUX_ELEMENTS_H
#define AUX_ELEMENTS_H

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
	
/**
 * Simpliest form of a control connecting one input with one output.
 */
class AuxElement : public JackMix::MixingMatrix::Element
                 , public dB2VolCalc {
Q_OBJECT
public:
	AuxElement( QStringList inchannel, QStringList outchannel, MixingMatrix::Widget*, const char* =0 );
	~AuxElement();
    MixingMatrix::Widget * p;
	int inchannels () const  { return 1; }
	int outchannels() const { return 1; }
	void setIndicator(const QColor& c);

    // wether mute button was pushed
    bool is_mute = false;

    //value showed by knob's indicator 
    double indicator_value=-42;
    JackMix::GUI::Knob* _poti;
    //delete me!!!
    QPushButton* MuteButton;
    QPushButton* ZeroButton;
    //delete me!!!
  

public slots:
	void emitvalue( double );
    //delete me!!!
    void slot_mute_channel();
    void slot_zero_channel();
    //delete me!!!
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void setKnobPointer_slot(double volume);
    void setUnityMute_slot(bool is_mute);
    void setUnityZero_slot();
    void removeThis();

};

void init_aux_elements();

}; // MixerElements
}; // JackMix

#endif // AUX_ELEMENTS_H

