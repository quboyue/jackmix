/*
    Copyright ( C ) 2004 Arnold Krille <arnold@arnoldarts.de>

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

#include "qfloatpoti.h"
#include "qfloatslider.h"
#include "slider.h"
#include "jack_backend.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qobjectlist.h>
#include <qlabel.h>

using namespace JackMix;
using namespace MixerElements;
using namespace MixingMatrix;

/// The Factory for creating this things...
class StereoFactory : public ElementFactory
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
StereoFactory* _stereofactory = new StereoFactory();


Mono2StereoElement::Mono2StereoElement( QStringList inchannel, QStringList outchannels, Widget* p, const char* n )
	: Element( inchannel, outchannels, p, n )
	, dB2VolCalc( -42, 6 )
	, _inchannel( inchannel[ 0 ] )
	, _outchannel1( outchannels[ 0 ] )
	, _outchannel2( outchannels[ 1 ] )
	, _balance_value( 0 )
	, _volume_value( 0 )
{
	float left = BACKEND->getVolume( _inchannel, _outchannel1 );
	float right = BACKEND->getVolume( _inchannel, _outchannel2 );
	if ( left>right ) {
		_volume_value = left;
		_balance_value = left-right;
	} else {
		_volume_value = right;
		_balance_value = left-right;
	}
	QGridLayout* _layout = new QGridLayout( this, 4,2, 3 );

	QPushButton* _btn_select = new QPushButton( "S", this );
	_layout->addWidget( _btn_select, 0,0 );
	connect( _btn_select, SIGNAL( clicked() ), this, SLOT( slot_toggle() ) );
	QPushButton* _btn_replace = new QPushButton( "R", this );
	_layout->addWidget( _btn_replace, 0,1 );
	connect( _btn_replace, SIGNAL( clicked() ), this, SLOT( slot_replace() ) );
	QPushButton* _btn_print = new QPushButton( "P", this );
	_layout->addWidget( _btn_print, 1,0 );
	connect( _btn_print, SIGNAL( clicked() ), this, SLOT( printSignals() ) );

	QFloatPoti* _balance = new QFloatPoti( _balance_value, -1, 1, 100, QColor( 0,0,255 ), this );
	_layout->addMultiCellWidget( _balance, 2,2, 0,1 );
	connect( _balance, SIGNAL( valueChanged( float ) ), this, SLOT( balance( float ) ) );
	JackMix::GUI::Slider* _volume = new JackMix::GUI::Slider( amptodb( _volume_value ), dbmin, dbmax, 1, 3, this );
	_layout->addMultiCellWidget( _volume, 3,3, 0,1 );
	connect( _volume, SIGNAL( valueChanged( float ) ), this, SLOT( volume( float ) ) );
}
Mono2StereoElement::~Mono2StereoElement() {
}

void Mono2StereoElement::slot_toggle() { select( !isSelected() ); }

void Mono2StereoElement::balance( float n ) {
	//qDebug( "Mono2StereoElement::balance( float %f )", n );
	_balance_value = n;
	calculateVolumes();
}
void Mono2StereoElement::volume( float n ) {
	//qDebug( "Mono2StereoElement::volume( float %f )", n );
	_volume_value = n;
	calculateVolumes();
}

void Mono2StereoElement::calculateVolumes() {
	float left, right;
		left = dbtoamp( _volume_value );
		right = dbtoamp( _volume_value );
	if ( _balance_value > 0 )
		left = dbtoamp( _volume_value )*( 1-_balance_value );
	if ( _balance_value < 0 )
		right = dbtoamp( _volume_value )*( 1+_balance_value );
	BACKEND->setVolume( _inchannel, _outchannel1, left );
	BACKEND->setVolume( _inchannel, _outchannel2, right );
}

void Mono2StereoElement::printSignals() {
	qDebug( "\n* Poperties: *" );
	QObjectListIterator it( * parent()->children() );
	while ( QObject* object = it.current() )  {
		++it;
		if ( strcmp( object->className(), "JackMix::MixerElements::AuxElement" ) ) {
		qDebug( "  Child %s (%s)", object->name(), object->className() );
			qDebug( "  with %i ins (%s) and %i outs (%s).", object->property( "ins" ).toInt(), object->property( "in" ).toStringList().join( "," ).latin1(), object->property( "outs" ).toInt(), object->property( "out" ).toStringList().join( "," ).latin1() );
			qDebug( "    Properties:" );
			QStrList tmp = object->metaObject()->propertyNames( false );
			tmp.sort();
			for ( uint i=0; i<tmp.count(); i++ ) {
				QString signal = tmp.at( i )+QString( "_changed(" ) + object->metaObject()->property( i )->type() + ")";
				QString slot = QString( "set_" ) + tmp.at( i ) + "(" + object->metaObject()->property( i )->type() + ")";
				//qDebug( "? signal: %s slot: %s ?", signal.latin1(), slot.latin1() );
				qDebug( "      %s (signal: %i, slot: %i)", tmp.at( i ),
					object->metaObject()->signalNames().contains( signal.latin1() ),
					object->metaObject()->slotNames().contains( slot.latin1() ) );
			}
		} else
		qDebug( "  Child %s (%s) - ignoring...", object->name(), object->className() );
	}
	qDebug( "\n" );
}

Stereo2StereoElement::Stereo2StereoElement( QStringList inchannels, QStringList outchannels, Widget* p, const char* n )
	: Element( inchannels, outchannels, p, n )
	, dB2VolCalc( -42, 6 )
	, _inchannel1( inchannels[ 0 ] )
	, _inchannel2( inchannels[ 1 ] )
	, _outchannel1( outchannels[ 0 ] )
	, _outchannel2( outchannels[ 1 ] )
	, _balance_value( 0 )
	, _volume_value( 0 )
{
	float left = BACKEND->getVolume( _inchannel1, _outchannel1 );
	float right = BACKEND->getVolume( _inchannel2, _outchannel2 );
	if ( left>right ) {
		_volume_value = left;
		_balance_value = left-right;
	} else {
		_volume_value = right;
		_balance_value = left-right;
	}
	QGridLayout* _layout = new QGridLayout( this, 3,2, 3 );
	QPushButton* _btn_select = new QPushButton( "S", this );
	_layout->addWidget( _btn_select, 0,0 );
	connect( _btn_select, SIGNAL( clicked() ), this, SLOT( slot_toggle() ) );
	QPushButton* _btn_replace = new QPushButton( "R", this );
	_layout->addWidget( _btn_replace, 0,1 );
	connect( _btn_replace, SIGNAL( clicked() ), this, SLOT( slot_replace() ) );
	_layout->setRowStretch( 0, -100 );
	JackMix::GUI::Slider* _balance_widget = new JackMix::GUI::Slider( _balance_value, -1, 1, 2, 0.1, this, "%1" );
	_layout->addMultiCellWidget( _balance_widget, 1,1, 0,1 );
	_layout->setRowStretch( 1, -100 );
	connect( _balance_widget, SIGNAL( valueChanged( float ) ), this, SLOT( balance( float ) ) );
	JackMix::GUI::Slider* _volume_widget = new JackMix::GUI::Slider( amptodb( _volume_value ), dbmin, dbmax, 1, 3, this );
	_layout->addMultiCellWidget( _volume_widget, 2,2, 0,1 );
	_layout->setRowStretch( 2, 1000 );
	connect( _volume_widget, SIGNAL( valueChanged( float ) ), this, SLOT( volume( float ) ) );
	BACKEND->setVolume( _inchannel1, _outchannel2, 0 );
	BACKEND->setVolume( _inchannel2, _outchannel1, 0);
}
Stereo2StereoElement::~Stereo2StereoElement() {
}

void Stereo2StereoElement::slot_toggle() { select( !isSelected() ); }

void Stereo2StereoElement::balance( float n ) {
	//qDebug( "Mono2StereoElement::balance( float %f )", n );
	_balance_value = n;
	calculateVolumes();
}
void Stereo2StereoElement::volume( float n ) {
	//qDebug( "Mono2StereoElement::volume( float %f )", n );
	_volume_value = n;
	calculateVolumes();
}

void Stereo2StereoElement::calculateVolumes() {
	float left, right;
		left = dbtoamp( _volume_value );
		right = dbtoamp( _volume_value );
	if ( _balance_value > 0 )
		left = dbtoamp( _volume_value )*( 1-_balance_value );
	if ( _balance_value < 0 )
		right = dbtoamp( _volume_value )*( 1+_balance_value );
	BACKEND->setVolume( _inchannel1, _outchannel1, left );
	BACKEND->setVolume( _inchannel2, _outchannel2, right );
}
