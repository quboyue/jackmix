

#include "volume_slider.h"
#include "volume_slider.moc"

#include <QtGui/QPainter>
#include <QtWidgets/QStyleOption>
#include <QtWidgets/QStyle>
#include <QtGui/QMouseEvent>
#include <QtCore/QDebug>
#include <QtCore/QTimer>

using namespace JackMix;
using namespace JackMix::GUI;





volume_bar::volume_bar(): QWidget(), dB2VolCalc(-42, 6)
{
	layout = new  QHBoxLayout(this);
	layout->setMargin(2);
	layout->setSpacing(2);

	qDebug() << "  THIS IS VOLUME BAR";

}
volume_bar::~volume_bar() {
}

void volume_bar::receive_OutputVolume(QString which, float max) {

	if (!_outchannels.contains(which))
		_outchannels << which;
	if (_volume_sliders.size() < _outchannels.size()) {
		volume_slider* one_slider = new volume_slider(0, -20, 3, 2, 1.0, 0,which);
		_volume_sliders.append(one_slider);
		layout->addWidget(one_slider, 0);
	}

	for (int i = 0; i < _volume_sliders.size(); i++) {
		if (_volume_sliders[i]->_name == which) {
			_volume_sliders[i]->_value = amptodb(max);
			_volume_sliders[i]->update();
		}

	}

	//qDebug() << "  _outchannels WHICH "<< which;
	//_value = amptodb(max);
	//update();


}












volume_slider::volume_slider(double value, double min, double max, int precision, double pagestep, QWidget* p ,QString name)
	: QWidget(p)
	, dB2VolCalc(-42, 6)
{
	QString valuestring = "%1 db";
	_name = name;
	setAutoFillBackground(false);
	int m = QFontMetrics(font()).height();
	int w = QFontMetrics(font()).width(valuestring);
	setMinimumSize(int(w ), int(m * 2.2));
	setFocusPolicy(Qt::TabFocus);


}
volume_slider::~volume_slider() {
}



#define SLIDER_BORDER 5

void volume_slider::paintEvent(QPaintEvent*) {
	//qDebug() << "    volume_slider::paintEvent	"<< _value;


	//qDebug() << " 	float max    newOutputLevel   max:::" << calcu_part->amptodb(max);

	//0, 0.00316, 0.12589, 0.50119
	// Thresholds are at -50dB, -18dB and -6dB
	//const float BackendInterface::threshold[] = { 0, 0.00316, 0.12589, 0.50119 };
	QStyleOption opt;
	QPainter p(this);
	p.setRenderHints(QPainter::Antialiasing);


	// Center the coordinates
	p.translate(width() / 2, height() / 2);



	double w = height() - SLIDER_BORDER;
	double h = width() - 4*SLIDER_BORDER;
	p.rotate(-90);


	if (hasFocus()) {
		style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opt, &p, this);
	}

	//double pos = dbtondb( _value )*w-w/2;
	QRectF bar(-w / 2, -h / 4, w, h / 2);

	// Tickmarks
	p.save();
	p.setPen(palette().color(QPalette::ButtonText));
	QFont small = font();
	small.setPointSizeF(qMax(qreal(5.0), font().pointSizeF() / 2.0));
	p.setFont(small);

	double _pagestep = 1;
	double dbmax = 15;
	double dbmin = -42;


		for (double a = _pagestep; a < dbmax; a += _pagestep)
			p.drawLine(QPointF(-w / 2 + w * dbtondb(a), h / 3.5), QPointF(-w / 2 + w * dbtondb(a), -h / 3.5));
		for (double a = -_pagestep; a > dbmin; a -= _pagestep)
			p.drawLine(QPointF(-w / 2 + w * dbtondb(a), h / 3.5), QPointF(-w / 2 + w * dbtondb(a), -h / 3.5));

	QList<double> _texts;
	_texts << dbmin << dbmax;
	if (0 > dbmin && 0 < dbmax)
		_texts << 0.0;
	foreach(double a, _texts) {
		p.save();
		p.translate(-w / 2 + w * dbtondb(a), 0);
		p.drawLine(QPointF(0, h / 3), QPointF(0, -h / 3));
		QRectF rect(
			0, 0,
			QFontMetrics(small).width("%1 dB"),
			QFontMetrics(small).height());

			p.rotate(90);
			if (dbtondb(a) < 0.5)
				rect.translate(0, -QFontMetrics(small).height());
			p.drawText(rect.translated(h / 3, 0), Qt::AlignCenter, QString("%1 dB").arg(a));
			//p.drawText( rect.translated( -h/3 -rect.width(), 0 ), Qt::AlignCenter, QString( _valuestring ).arg( a ) );
			//if (a == 0.0) {
				//_nullclick = p.matrix().mapRect(rect.translated(h / 3, 0)).toRect();
				//_nullclick = _nullclick.united( p.matrix().mapRect( rect.translated( -h/3-rect.width(),0 ) ).toRect() );
			//}
		
		p.restore();
	}
	p.restore();

	// Surrounding rect
	p.drawRect(bar);

	int barTopColor_RedChannel;

	barTopColor_RedChannel = int((60/(dbmax - dbmin))*_value);

	// Rect for the whole bar
	{
		p.save();
		QLinearGradient grad(QPointF(-w / 2, -h / 4), QPointF(w / 2, -h / 4));
		// Global ends first
		grad.setColorAt(0.0, QColor(0, 215, 0));//bottom
		if (dbtondb(_value) < 1.0)
			grad.setColorAt(1.0, QColor(255, 255, 255));//top

		// Next soft-fades
		grad.setColorAt(qMax(0.0, dbtondb(_value) - 0.01), QColor(barTopColor_RedChannel+190, 215, 0));
		if (dbtondb(_value) + 0.01 < 1.0)
			grad.setColorAt(qMin(1.0,dbtondb(_value) + 0.01), QColor(230, 230, 230));


		// Last the value itself
		grad.setColorAt(dbtondb(_value), QColor(barTopColor_RedChannel + 190,215, 0));
		// That way minimum and maximum get the right color

		p.fillRect(bar, grad);
		p.restore();
	}

	// Set _faderarea correctly
	_faderarea = p.matrix().mapRect(bar).toRect();



	// Draw the value as text in the lower third

}





