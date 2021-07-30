

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

volume_slider::volume_slider(double value, double min, double max, int precision, double pagestep, QWidget* p, QString valuestring)
	: AbstractSlider(value, min, max, precision, pagestep, p, valuestring)
	, _timer(new QTimer(this))
	, _show_value(false)
{
	setAutoFillBackground(false);
	int m = QFontMetrics(font()).height();
	int w = QFontMetrics(font()).width(valuestring);
	setMinimumSize(int(w ), int(m * 2.2));
	setFocusPolicy(Qt::TabFocus);

	_timer->setInterval(2000);
	_timer->setSingleShot(true);
	connect(_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}
volume_slider::~volume_slider() {
}

void volume_slider::value(double n, bool show_numeric) {
	if (!_value_inupdate) {
		AbstractSlider::value(n);
		if (show_numeric) {
			_show_value = true;
			_timer->start();
		}
	}
}
void volume_slider::timeout() {
	_show_value = false;
	update();
}

#define SLIDER_BORDER 2

void volume_slider::paintEvent(QPaintEvent*) {
	qDebug() << "    volume_slider::paintEvent";
	//0, 0.00316, 0.12589, 0.50119
	// Thresholds are at -50dB, -18dB and -6dB
	//const float BackendInterface::threshold[] = { 0, 0.00316, 0.12589, 0.50119 };
	QStyleOption opt;
	QPainter p(this);
	p.setRenderHints(QPainter::Antialiasing);
	QString tmp = QString::number(_value);
	if (tmp.contains(".")) tmp = _valuestring.arg(tmp.left(tmp.indexOf(".") + _precision + 1));
	else tmp = _valuestring.arg(tmp);

	QFontMetrics metrics(font());
	int fontwidth = metrics.width(tmp);

	// Center the coordinates
	p.translate(width() / 2, height() / 2);

	double w = width() - SLIDER_BORDER;
	double h = height() - SLIDER_BORDER;

	w = height() - SLIDER_BORDER;
	h = width() - SLIDER_BORDER;
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
	if (_show_value) {
		for (double a = _pagestep; a < dbmax; a += _pagestep)
			p.drawLine(QPointF(-w / 2 + w * dbtondb(a), h / 3.5), QPointF(-w / 2 + w * dbtondb(a), -h / 3.5));
		for (double a = -_pagestep; a > dbmin; a -= _pagestep)
			p.drawLine(QPointF(-w / 2 + w * dbtondb(a), h / 3.5), QPointF(-w / 2 + w * dbtondb(a), -h / 3.5));
	}
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
			QFontMetrics(small).width(_valuestring),
			QFontMetrics(small).height());

			p.rotate(90);
			if (dbtondb(a) < 0.5)
				rect.translate(0, -QFontMetrics(small).height());
			p.drawText(rect.translated(h / 3, 0), Qt::AlignCenter, QString(_valuestring).arg(a));
			//p.drawText( rect.translated( -h/3 -rect.width(), 0 ), Qt::AlignCenter, QString( _valuestring ).arg( a ) );
			if (a == 0.0) {
				_nullclick = p.matrix().mapRect(rect.translated(h / 3, 0)).toRect();
				//_nullclick = _nullclick.united( p.matrix().mapRect( rect.translated( -h/3-rect.width(),0 ) ).toRect() );
			}
		
		p.restore();
	}
	p.restore();

	// Surrounding rect
	p.drawRect(bar);

	// Rect for the whole bar
	{
		p.save();
		QLinearGradient grad(QPointF(-w / 2, -h / 4), QPointF(w / 2, -h / 4));
		// Global ends first
		grad.setColorAt(0.0, QColor(0, 255, 0));
		if (dbtondb(_value) < 1.0)
			grad.setColorAt(1.0, QColor(255, 255, 255));

		// Next soft-fades
		grad.setColorAt(qMax(0.0, dbtondb(_value) - 0.01), QColor(255, 0, 0));
		if (dbtondb(_value) + 0.01 < 1.0)
			grad.setColorAt(qMin(1.0, dbtondb(_value) + 0.01), QColor(255, 0, 255));


		// Last the value itself
		grad.setColorAt(dbtondb(_value), QColor(0, 0, 255));
		// That way minimum and maximum get the right color

		p.fillRect(bar, grad);
		p.restore();
	}

	// Set _faderarea correctly
	_faderarea = p.matrix().mapRect(bar).toRect();



	// Draw the value as text in the lower third
	if (_show_value) {
		p.save();
		p.setPen(Qt::NoPen);
		p.setBrush(QColor(255, 0, 0));
		p.setOpacity(0.75);
		p.drawRoundRect(-fontwidth / 2 - 2, height() / 3 - metrics.ascent() - 1, fontwidth + 4, metrics.ascent() + 4);
		p.restore();
		// Text showing the value
		p.drawText(-fontwidth / 2, height() / 3, tmp);
	}
}


void volume_slider::mouseEvent(QMouseEvent* ev) {
	/*
	if (width() >= height())
		value(ndbtodb(
			(ev->pos().x() - _faderarea.x()) / double(_faderarea.width() - 1)
		));
	else
		value(ndbtodb(
			(_faderarea.height() - ev->pos().y() + _faderarea.y()) / double(_faderarea.height() - 1)
		));
		*/
}
