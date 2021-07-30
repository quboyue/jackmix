
/*
This is a demo used to add a new kind of elements which used to control unity gain and loss

*/

#include "unity_elements.h"
#include "unity_elements.moc"



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
class UnityFactory : public JackMix::MixingMatrix::ElementFactory
{
public:
	UnityFactory() : ElementFactory() {
		//qDebug() << "				------------!!!!!!!!!!111    UnityFactory::UnityFactory()";
		globaldebug();
	}
	~UnityFactory() {}

	QStringList canCreate() const {
		return QStringList() << "UnityElement";
	}
	QStringList canCreate(int in, int out) const {
		qDebug() << "	canCreate UnityFactory::UnityFactory() in"<<in<<" "<<out;
		if (in == 0 && out == 0) {

			return QStringList() << "UnityElement";
		}
		return QStringList();

	}

	Element* create(QString type, QStringList ins, QStringList outs, Widget* p, const char* n = 0) {
		if (type == "UnityElement") {
			QStringList linshiin;
			QStringList linshiout;
			linshiin.push_back("in_1");
			linshiout.push_back("out_1");
			return new UnityElement(linshiin, linshiout, p, n);

		}
		else
			return 0;
	}
};

void MixerElements::init_unity_elements() {
	new UnityFactory();
}

UnityElement::UnityElement(QStringList inchannel, QStringList outchannel, MixingMatrix::Widget* p, const char* n)
	: Element(inchannel, outchannel, p, n)
	, dB2VolCalc(-42, 6)
{

	this->setMouseTracking(true);
	
	qDebug() << "UnityElement inchannel " << inchannel << " outchannel " << outchannel;



	QVBoxLayout* _layout = new QVBoxLayout(this);



	qDebug() << " _poti = new JackMix::GUI::Knob ";
	_poti = new JackMix::GUI::Knob(
		amptodb(backend()->getVolume(_in[0], _out[0])),
		dbmin, dbmax, 2, 3, this);
	_layout->addWidget(_poti, 100);

	//delete me!!!!
	MuteButton = new QPushButton();
	MuteButton->setStyleSheet("background-color: rgb(175,175,175)");
	MuteButton->setCheckable(true);
	_layout->addWidget(MuteButton, 1);
	MuteButton->setText("Unity Mute");
	MuteButton->setMouseTracking(true);
	connect(MuteButton, SIGNAL(toggled(bool)), this, SLOT(slot_mute_channel(bool)));

	//delete me!!!!


	connect(_poti, SIGNAL(valueChanged(double)), this, SLOT(emitvalue(double)));

	connect(_poti, SIGNAL(valueChanged(double)), this, SLOT(emitvalue(double)));

}
UnityElement::~UnityElement() {
}

void UnityElement::emitvalue(double n) {
	//setKnobPointer(_poti->_value);
	emit sendKnobPointer_signal(_poti->_value);
	return;
}

void UnityElement::setIndicator(const QColor& c) { qDebug() << "	setsetIndicatorColor "; };


//delete me!!!
void UnityElement::slot_mute_channel(bool input) {
	qDebug() << backend()->inchannels()<<"  backend()->inchannels ";

	return;
}


void UnityElement::mouseMoveEvent(QMouseEvent* event)
{

	return;
}

void UnityElement::mousePressEvent(QMouseEvent* event)
{

	return;


}
//delete me!!!