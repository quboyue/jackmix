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

#include "stereo_elements.h"
#include "stereo_elements.moc"

#include "knob.h"
#include "slider.h"
#include "midicontrolchannelassigner.h"

#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include <QtCore/QList>
#include <QtWidgets/QLabel>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>

#include <controlsender.h>

using namespace JackMix;
using namespace JackMix::MixerElements;
using namespace JackMix::MixingMatrix;

/// The Factory for creating this things...
class StereoFactory : public JackMix::MixingMatrix::ElementFactory
{
public:
	StereoFactory() : ElementFactory() { globaldebug(); }
	~StereoFactory() {}

	QStringList canCreate() const {
		return QStringList()<<"Mono2StereoElement"<<"Stereo2StereoElement";
	}
	QStringList canCreate( int in, int out ) const {
		if ( in==1 && out==2 ) return QStringList()<<"Mono2StereoElement";
		if ( in==2 && out==2 ) return QStringList()<<"Stereo2StereoElement";
		return QStringList();
	}

	Element* create( QString type , QStringList ins, QStringList outs, Widget* p, const char* n=0 ) {
		if ( type=="Mono2StereoElement" )
			return new Mono2StereoElement( ins, outs, p, n );
		if ( type=="Stereo2StereoElement" )
			return new Stereo2StereoElement( ins, outs, p, n );
		return 0;
	}
};

void MixerElements::init_stereo_elements() {
	new StereoFactory();
}


Mono2StereoElement::Mono2StereoElement( QStringList inchannel, QStringList outchannels, MixingMatrix::Widget* p, const char* n )
	: Element( inchannel, outchannels, p, n )
	, dB2VolCalc( -42, 6 )
	, _balance_value( 0 )
	, _volume_value( 0 )
{
	//qDebug( "Mono2StereoElement::Mono2StereoElement()" );
	double left = backend()->getVolume( _in[0], _out[0] );
	double right = backend()->getVolume( _in[0], _out[1] );
	//qDebug( " volumes: %f, %f", left, right );
	if ( left>right ) {
		_volume_value = left;
		_balance_value = right-left;
	} else {
		_volume_value = right;
		_balance_value = right-left;
	}
	//qDebug( " values: %f, %f", _volume_value, _balance_value );
	QVBoxLayout* _layout = new QVBoxLayout( this );
	_layout->setMargin( 0 );
	_layout->setSpacing( 0 );
  
	menu()->addAction( "Select", this, SLOT( slot_simple_select() ) );
	menu()->addAction( "Replace", this, SLOT( slot_simple_replace() ) );
    menu()->addAction( "Explode", this, SLOT( slot_simple_explode() ) );
	menu()->addAction( "Assign MIDI Parameter", this, SLOT( slot_assign_midi_parameters() ) );
	



	Explode_button = new QPushButton("X");
	Explode_button->setFixedSize(30, 30);
	_layout->addWidget(Explode_button, 0);
	connect(Explode_button, SIGNAL(clicked()), this, SLOT(slot_simple_explode()));


	_balance = new JackMix::GUI::Knob( _balance_value, -1, 1, 2, 0.1, this, "%1" );

	_layout->addWidget( _balance, 10 );

	QHBoxLayout* _layout_button = new QHBoxLayout(this);

	MuteButton_left = new QPushButton("Mute L");
	MuteButton_left->setFixedSize(80, 50);
	MuteButton_left->setStyleSheet("background-color: rgb(175,175,175)");
	MuteButton_left->setCheckable(true);
	MuteButton_left->setFont(QFont("", 7));
	connect(MuteButton_left, SIGNAL(toggled(bool)), this, SLOT(slot_mute_channel_left()));
	_layout_button->addWidget(MuteButton_left, 1);

	ZeroButton = new QPushButton("Zero");
	ZeroButton->setFixedSize(80, 50);
	ZeroButton->setStyleSheet("background-color: rgb(175,175,175)");
	ZeroButton->setFont(QFont("", 7));

	connect(ZeroButton, SIGNAL(clicked()), this, SLOT(slot_zero_channel()));
	_layout_button->addWidget(ZeroButton, 2);



	MuteButton_right = new QPushButton("Mute R");
	MuteButton_right->setFixedSize(80, 50);
	MuteButton_right->setStyleSheet("background-color: rgb(175,175,175)");
	MuteButton_right->setCheckable(true);
	MuteButton_right->setFont(QFont("", 7));
	connect(MuteButton_right, SIGNAL(toggled(bool)), this, SLOT(slot_mute_channel_right()));
	_layout_button->addWidget(MuteButton_right, 3);


	_layout->addLayout(_layout_button);





	connect( _balance, SIGNAL( valueChanged( double ) ), this, SLOT( balance( double ) ) );
	connect( _balance, SIGNAL( select() ), this, SLOT( slot_simple_select() ) );
	connect( _balance, SIGNAL( replace() ), this, SLOT( slot_simple_replace() ) );
	_volume = new JackMix::GUI::Slider( amptodb( _volume_value ), dbmin, dbmax, 2, 3, this );
	_layout->addWidget( _volume, 20 );


	connect( _volume, SIGNAL( valueChanged( double ) ), this, SLOT( volume( double ) ) );
	connect( _volume, SIGNAL( select() ), this, SLOT( slot_simple_select() ) );
	connect( _volume, SIGNAL( replace() ), this, SLOT( slot_simple_replace() ) );

	// WATCH OUT: Order of initialisation is really important!
	// Make sure all the widgets are contructed before adding them to the delegates list

	// Initial MIDI parameters and associated AbstractSliders
	midi_params.append(0);
	midi_delegates.append(_volume);
	midi_params.append(0);
	midi_delegates.append(_balance);

	// Now construct the parameter setting menu
	_cca = new JackMix::GUI::MidiControlChannelAssigner(QString("Set MIDI control parameter"),
	                                                     "<qt>" + _in[0] + " &rarr; ("  + _out[0] + "/" + _out[1] + ")</qt>",
	                                                     QStringList() << "Gain" << "Pan",
	                                                     midi_params,
	                                                     this
	                                                    );
	connect( _cca, SIGNAL(assignParameters(QList<int>)), this, SLOT(update_midi_parameters(QList<int>)) );

}
Mono2StereoElement::~Mono2StereoElement() {
}

void Mono2StereoElement::balance( double n ) {
	//qDebug( "Mono2StereoElement::balance( double %f )", n );
	_balance_value = n;
	calculateVolumes();
	_balance->value( n );
	emit valueChanged( this, QString( "balance" ) );
}
void Mono2StereoElement::volume( double n ) {
	//qDebug( "Mono2StereoElement::volume( double %f )", n );
	_volume_value = n;
	calculateVolumes();
	_volume->value( n );
	emit valueChanged( this, QString( "volume" ) );
}

void Mono2StereoElement::calculateVolumes() {
	double left, right;
		left = dbtoamp( _volume_value );
		right = dbtoamp( _volume_value );
	if ( _balance_value > 0 )
		left = dbtoamp( _volume_value )*( 1-_balance_value );
	if ( _balance_value < 0 )
		right = dbtoamp( _volume_value )*( 1+_balance_value );

	indicator_value_left = left;
	indicator_value_right = right;
	if (!is_mute_left)
		backend()->setVolume(_in[0], _out[0], left);
	if (!is_mute_right)
		backend()->setVolume(_in[0], _out[1], right);

}

void Mono2StereoElement::slot_mute_channel_left() {


	if (is_mute_left) {

		MuteButton_left->setChecked(false);
		is_mute_left = false;
		qDebug() << " indicator_value " << indicator_value_left;
		backend()->setVolume(_in[0], _out[0], indicator_value_left);
	}
	else {
		MuteButton_left->setChecked(true);
		is_mute_left = true;
		backend()->setVolume(_in[0], _out[0], dbtoamp(-42));
	}

}

void Mono2StereoElement::slot_mute_channel_right() {


	if (is_mute_right) {
		MuteButton_right->setChecked(false);
		is_mute_right = false;
		qDebug() << " indicator_value " << indicator_value_right;
		backend()->setVolume(_in[0], _out[1], indicator_value_right);
	}
	else {
		MuteButton_right->setChecked(true);
		is_mute_right = true;
		backend()->setVolume(_in[0], _out[1], dbtoamp(-42));
	}

}
void Mono2StereoElement::setKnobPointer_slot(double volume) {
	
		//qDebug() << "Mono2StereoElement setKnobPointer_slot";

		_volume_value = volume;
		calculateVolumes();
		_volume->value(volume);
		emit valueChanged(this, QString("volume"));
		_volume->update();

}
void Mono2StereoElement::setUnityMute_slot(bool is_mute) {
	if (is_mute != this->is_mute_left) {
		slot_mute_channel_left();
	}
	if (is_mute != this->is_mute_right) {
		slot_mute_channel_right();
	}


}



void Mono2StereoElement::slot_zero_channel (){


	_volume_value = 0;
	calculateVolumes();
	_volume->value(0);
	emit valueChanged(this, QString("volume"));
	_volume->update();

	MuteButton_right->setChecked(false);
	is_mute_right = false;
	MuteButton_left->setChecked(false);
	is_mute_left = false;
	//qDebug() << " indicator_value " << indicator_value_right;
	//backend()->setVolume(_in[0], _out[1], indicator_value_right);
}







Stereo2StereoElement::Stereo2StereoElement( QStringList inchannels, QStringList outchannels, MixingMatrix::Widget* p, const char* n )
	: Element( inchannels, outchannels, p, n )
	, dB2VolCalc( -42, 6 )
	, _balance_value( 0 )
	, _volume_value( 0 )
{
	backend()->setVolume( _in[0], _out[1], 0 );
	backend()->setVolume( _in[1], _out[0], 0 );
	double left = backend()->getVolume( _in[0], _out[0] );
	double right = backend()->getVolume( _in[1], _out[1] );
	if ( left>right )
		_volume_value = left;
	else
		_volume_value = right;
	_balance_value = qMin( qMax( right-left, -1.0 ), 1.0 );
	QGridLayout* _layout = new QGridLayout( this );
	_layout->setSpacing( 1 );
	_layout->setMargin( 2 );


	Explode_button = new QPushButton("X");
	Explode_button->setFixedSize(30, 30);
	_layout->addWidget(Explode_button, 0,0,1,1);
	connect(Explode_button, SIGNAL(clicked()), this, SLOT(slot_simple_explode()));



	MuteButton_left = new QPushButton("Mute L");
	MuteButton_left->setFixedSize(80, 50);
	MuteButton_left->setStyleSheet("background-color: rgb(175,175,175)");
	MuteButton_left->setCheckable(true);
	MuteButton_left->setFont(QFont("", 7));
	connect(MuteButton_left, SIGNAL(toggled(bool)), this, SLOT(slot_mute_channel_left()));
	_layout->addWidget(MuteButton_left, 2,0,1,1);


	ZeroButton = new QPushButton("Zero");
	ZeroButton->setFixedSize(80, 50);
	ZeroButton->setStyleSheet("background-color: rgb(175,175,175)");
	ZeroButton->setFont(QFont("", 7));
	connect(ZeroButton, SIGNAL(clicked()), this, SLOT(slot_zero_channel()));
	_layout->addWidget(ZeroButton, 2,1,1,1);


	MuteButton_right = new QPushButton("Mute R");
	MuteButton_right->setFixedSize(80, 50);
	MuteButton_right->setStyleSheet("background-color: rgb(175,175,175)");
	MuteButton_right->setCheckable(true);
	MuteButton_right->setFont(QFont("", 7));
	connect(MuteButton_right, SIGNAL(toggled(bool)), this, SLOT(slot_mute_channel_right()));
	_layout->addWidget(MuteButton_right, 2, 2, 1, 1);






	_balance_widget = new JackMix::GUI::Slider( _balance_value, -1, 1, 2, 0.1, this, "%1" );
	_layout->addWidget( _balance_widget, 1,0,1,3);
	//_layout->setRowStretch( 0, 0);
	connect( _balance_widget, SIGNAL( valueChanged( double ) ), this, SLOT( balance( double ) ) );
	connect( _balance_widget, SIGNAL( select() ), this, SLOT( slot_simple_select() ) );
	connect( _balance_widget, SIGNAL( replace() ), this, SLOT( slot_simple_replace() ) );
	_volume_widget = new JackMix::GUI::Slider( amptodb( _volume_value ), dbmin, dbmax, 1, 3, this );
	_layout->addWidget( _volume_widget, 3,0,1,3);
	//_layout->setRowStretch( 1, 255 );
	connect( _volume_widget, SIGNAL( valueChanged( double ) ), this, SLOT( volume( double ) ) );
	connect( _volume_widget, SIGNAL( select() ), this, SLOT( slot_simple_select() ) );
	connect( _volume_widget, SIGNAL( replace() ), this, SLOT( slot_simple_replace() ) );

	QAction *toggle = new QAction( "Toggle Selection", this );
	connect( toggle, SIGNAL( triggered() ), this, SLOT( slot_simple_select() ) );
	menu()->addAction( toggle );
	QAction *replace = new QAction( "Replace", this );
	connect( replace, SIGNAL( triggered() ), this, SLOT( slot_simple_replace() ) );
	menu()->addAction( replace );
	QAction *explode = new QAction( "Explode", this );
	connect( explode, SIGNAL( triggered() ), this, SLOT( slot_simple_explode() ) );
	menu()->addAction( explode );

	QAction *assign = new QAction( "Assign MIDI Parameter", this );
	connect( assign, SIGNAL( triggered() ), this, SLOT( slot_assign_midi_parameters() ) );
	menu()->addAction( assign );
	
	// WATCH OUT: Order of initialisation is really important!
	// Make sure all the widgets are contructed before adding them to the delegates list

	// Initial MIDI parameters and associated AbstractSliders
	midi_params.append(0);
	midi_delegates.append(_volume_widget);
	midi_params.append(0);
	midi_delegates.append(_balance_widget);

	// Now construct the parameter setting menu
	_cca = new JackMix::GUI::MidiControlChannelAssigner(QString("Set MIDI control parameter"),
	                                                     "<qt>(" + _in[0] + "/" + _in[1] +
	                                                         ") &rarr; ("  + _out[0] + "/" + _out[1] + ")</qt>",
	                                                     QStringList() << "Gain" << "Cross-fade",
	                                                     midi_params,
	                                                     this
	                                                    );
	connect( _cca, SIGNAL(assignParameters(QList<int>)), this, SLOT(update_midi_parameters(QList<int>)) );


}
Stereo2StereoElement::~Stereo2StereoElement() {
}

void Stereo2StereoElement::balance( double n ) {
	//qDebug( "Mono2StereoElement::balance( double %f )", n );
	_balance_value = n;
	_balance_widget->value( n );
	calculateVolumes();
	emit valueChanged( this, QString( "balance" ) );
}
void Stereo2StereoElement::volume( double n ) {
	//qDebug( "Mono2StereoElement::volume( double %f )", n );
	_volume_value = n;
	_volume_widget->value( n );
	calculateVolumes();
	emit valueChanged( this, QString( "volume" ) );
}

void Stereo2StereoElement::calculateVolumes() {
	double left, right;
		left = dbtoamp( _volume_value );
		right = dbtoamp( _volume_value );
	if ( _balance_value > 0 )
		left = dbtoamp( _volume_value )*( 1-_balance_value );
	if ( _balance_value < 0 )
		right = dbtoamp( _volume_value )*( 1+_balance_value );

	indicator_value_left = left;
	indicator_value_right = right;
	if (!is_mute_left)
		backend()->setVolume( _in[0], _out[0], left );
	if (!is_mute_right)
		backend()->setVolume( _in[1], _out[1], right );
}


void Stereo2StereoElement::slot_mute_channel_left( ) {


	if (is_mute_left) {
		MuteButton_left->setChecked(false);
		is_mute_left = false;
		qDebug() << " indicator_value " << indicator_value_left;
		backend()->setVolume(_in[0], _out[0], indicator_value_left);
	}
	else {
		MuteButton_left->setChecked(true);
		is_mute_left = true;
		backend()->setVolume(_in[0], _out[0], dbtoamp(-42));
	}

}

void Stereo2StereoElement::slot_mute_channel_right( ) {


	if (is_mute_right) {
		MuteButton_right->setChecked(false);
		is_mute_right = false;
		qDebug() << " indicator_value " << indicator_value_right;
		backend()->setVolume(_in[1], _out[1], indicator_value_right);
	}
	else {
		MuteButton_right->setChecked(true);
		is_mute_right = true;
		backend()->setVolume(_in[1], _out[1], dbtoamp(-42));
	}

}
void Stereo2StereoElement::setKnobPointer_slot(double volume) {

	//qDebug() << "Stereo2StereoElement setKnobPointer_slot";

	_volume_value = volume;
	_volume_widget->value(volume);
	calculateVolumes();
	emit valueChanged(this, QString("volume"));
	//update();

}

void Stereo2StereoElement::setUnityMute_slot(bool is_mute) {
	if (is_mute != this->is_mute_left) {
		slot_mute_channel_left();
	}
	if (is_mute != this->is_mute_right) {
		slot_mute_channel_right();
	}


}


void Stereo2StereoElement::slot_zero_channel() {
	_volume_value = 0;
	_volume_widget->value(0);
	calculateVolumes();
	emit valueChanged(this, QString("volume"));


	MuteButton_right->setChecked(false);
	is_mute_right = false;
	MuteButton_left->setChecked(false);
	is_mute_left = false;


}