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

#include "aux_elements.h"
#include "aux_elements.moc"

#include "knob.h"
#include "midicontrolchannelassigner.h"

#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtCore/QList>
#include <QtCore/QStringList>
//#include <QtCore/QDebug>

#include <controlreceiver.h>
#include <controlsender.h>
#include <mixingmatrix.h>


using namespace JackMix;
using namespace JackMix::MixerElements;
using namespace JackMix::MixingMatrix;

/// The Factory for creating this things...
class AuxFactory : public JackMix::MixingMatrix::ElementFactory
{
public:
	AuxFactory() : ElementFactory() {
		//qDebug() << "AuxFactory::AuxFactory()";
		//globaldebug();
	}
	~AuxFactory() {}

	QStringList canCreate() const {
		return QStringList()<<"AuxElement";
	}
	QStringList canCreate( int in, int out ) const {
		if ( in==1 && out==1 ) return QStringList()<<"AuxElement";
		return QStringList();
	}

	Element* create( QString type , QStringList ins, QStringList outs, Widget* p, const char* n=0 ) {
		if ( type=="AuxElement" )
			return new AuxElement( ins, outs, p, n );
		else
			return 0;
	}
};

void MixerElements::init_aux_elements() {
	new AuxFactory();
}

AuxElement::AuxElement( QStringList inchannel, QStringList outchannel, MixingMatrix::Widget* p, const char* n )
	: Element( inchannel, outchannel, p, n )
	, dB2VolCalc( -42, 6 )
{

	this->p = p;
	//this->setStyleSheet("border:2px solid red;");
	this->setMouseTracking(true);
	//qDebug( "AuxElement( QStringList '%s', QStringList '%s', %s)", qPrintable(inchannel.join( "," ) ), qPrintable(outchannel.join( "," ) ) );
	qDebug() << "AuxElement inchannel " << inchannel<< " outchannel " << outchannel;

	//if (p->mode() == Widget::Select) {
	//	menu()->addAction( "Select", this, SLOT( slot_simple_select() ) );
	//	menu()->addAction( "Replace", this, SLOT( slot_simple_replace() ) );
	//}




	QVBoxLayout* _layout = new QVBoxLayout( this );

	if ( _in[0] == _out[0] ) {
		menu()->addAction("Remove", this, SLOT(removeThis()));
		menu()->addAction("Add new port", p, SLOT(addNew()));
		disp_name = new QLabel( QString( "<qt><center>%1</center></qt>" ).arg( _in[0] ), this );
		_layout->addWidget(disp_name, 0);
	}
	menu()->addAction("Assign MIDI Parameter", this, SLOT(slot_assign_midi_parameters()));

	qDebug() << " _poti = new JackMix::GUI::Knob ";
	_poti = new JackMix::GUI::Knob(
		amptodb( backend()->getVolume( _in[0], _out[0] ) ),
		dbmin, dbmax, 2, 3, this );
	_layout->addWidget( _poti, 100 );
	//delete me!!!!
	MuteButton = new QPushButton();
	MuteButton->setStyleSheet("background-color: rgb(175,175,175)");
	MuteButton->setCheckable(true);
	_layout->addWidget(MuteButton, 1);
	MuteButton->setText("Mute");
	MuteButton->setMouseTracking(true);
	_poti->setMinimumSize(150, 90);
	MuteButton->setMinimumSize(20,30);
	connect(MuteButton, SIGNAL(toggled(bool)), this, SLOT(slot_mute_channel()));
	connect(_poti, SIGNAL(replace(QString)), this, SLOT(slot_simple_replace(QString)));
	//delete me!!!!


	connect( _poti, SIGNAL( valueChanged( double ) ), this, SLOT( emitvalue( double ) ) );



	midi_params.append(0);        // Initial MIDI paramater number
	midi_delegates.append(_poti); //   for the potentiometer
	//qDebug()<<"There are "<<midi_delegates.size()<<" Midi delegates";

	// Now construct the parameter setting menu
	_cca = new JackMix::GUI::MidiControlChannelAssigner(QString("Set MIDI control parameter"),
		"<qt>" + _in[0] + " &rarr; " + _out[0] + "</qt>",
		QStringList() << "Gain",
		midi_params,
		this
	);
	connect(_cca, SIGNAL(assignParameters(QList<int>)), this, SLOT(update_midi_parameters(QList<int>)));


}
AuxElement::~AuxElement() {
}

void AuxElement::emitvalue( double n ) {
	qDebug() << " backend()->setVolume" << _in << "  " << _out<<" is_mute" << is_mute<<"  "<< n;
	if(!is_mute)
		backend()->setVolume(_in[0], _out[0], dbtoamp(n));

}

void AuxElement::setIndicator(const QColor& c) { _poti->setIndicatorColor(c); };


//delete me!!!
void AuxElement::slot_mute_channel() {




	if (is_mute) 
	{
		MuteButton->setChecked(false);
		is_mute = false;
		qDebug() << " indicator_value " << _poti->_value;
		backend()->setVolume(_in[0], _out[0], dbtoamp(_poti->_value));
	}
	else 
	{	
		MuteButton->setChecked(true);
		is_mute = true;
		backend()->setVolume(_in[0], _out[0], dbtoamp(-42));
	}


}


void AuxElement::mouseMoveEvent(QMouseEvent* event)
{
	if (_in[0] == _out[0]) 
		return;

	QPoint p_ab = event->globalPos();
	int mouse_x = mapFromGlobal(p_ab).x();
	int mouse_y = mapFromGlobal(p_ab).y();
	int mute_button_bottom = MuteButton->frameGeometry().y() + MuteButton->frameGeometry().height() + 1;
	int mute_button_right = MuteButton->frameGeometry().x() + MuteButton->frameGeometry().width() + 1;
	if (mouse_y<this->height() && mouse_y> mute_button_bottom) {
		setCursor(Qt::SplitVCursor);
		if (mouse_x < this->width() && mouse_x>mute_button_right ) {
			setCursor(Qt::SizeAllCursor);
		}
	}
	else
		setCursor(Qt::ArrowCursor);
	

	//if (cursor().shape() == Qt::SplitVCursor)
}

void AuxElement::mousePressEvent(QMouseEvent* event)
{
	
	if (event->button() == Qt::LeftButton  )
	{

		if (cursor().shape() == Qt::SplitVCursor)
			emit _poti->replace("mono");
		if (cursor().shape() == Qt::SizeAllCursor)
			emit _poti->replace("stereo");
	}

}
//delete me!!!



void  AuxElement::setKnobPointer_slot(double volume) {
	//qDebug() << "AuxElement setKnobPointer_slot";
	/*
	double real_volume;
	real_volume = volume + _poti->_value;
	if (real_volume > 6) {
		real_volume = 6;
	}
	if (real_volume < -42) {
		real_volume = -42;
	}
	*/

		emitvalue(volume);
		_poti->_value = volume;
		_poti->update();

}

void  AuxElement::setUnityMute_slot(bool is_mute) {
	if (is_mute != this->is_mute) {
		slot_mute_channel();
	}


}

void AuxElement::removeThis() {
	p->removeItem = _in[0];
	emit p->check_removeItem();
}