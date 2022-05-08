#include "macbethview.h"

#include <QPointF>
#include <QGraphicsTextItem>
#include <QColor>
#include <array>

MacbethView::MacbethView(QWidget* parent): QGraphicsView(parent), _model(nullptr)
{
    setScene(new QGraphicsScene);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QGraphicsTextItem* text = scene()->addText(tr("Loading..."));
    text->setScale(3);
    text->setDefaultTextColor(Qt::white);

    qRegisterMetaType<std::array<QColor, 24>>("std::array<QColor, 24>");
}


void MacbethView::setModel(MacbethModel* model)
{
    _model = model;

    connect(
      model,
      SIGNAL(macbethChanged(const std::array<QColor, 24>&)),
      this,
      SLOT(onMacbethChanged(const std::array<QColor, 24>&)));

    if (_model->ready()) onMacbethChanged(_model->getPatchesColors());
}

void MacbethView::resizeEvent(QResizeEvent*)
{
    fitInView(0, 0, _macbethWidth, _macbethHeight, Qt::KeepAspectRatio);
}

void MacbethView::onMacbethChanged(const std::array<QColor, 24>& colors)
{
    scene()->clear();

    scene()->addRect(QRectF(0, 0, _macbethWidth, _macbethHeight), QPen(), QBrush(Qt::black));

    const int n_cols  = 6;
    const int n_lines = 4;

    const float dead_width  = _macbethWidth * .1;
    const float dead_height = _macbethHeight * .1;

    const float margin_width  = dead_width / float(n_cols + 1);
    const float margin_height = dead_height / float(n_lines + 1);

    const float effective_width  = _macbethWidth - dead_width;
    const float effective_height = _macbethHeight - dead_height;

    const float patch_width  = effective_width / float(n_cols);
    const float patch_height = effective_height / float(n_lines);

    for (int y = 0; y < n_lines; y++) {
        for (int x = 0; x < n_cols; x++) {
            const float x_left = float(x + 1) * margin_width + x * patch_width;
            const float y_top  = float(y + 1) * margin_height + y * patch_height;

            scene()->addRect(QRectF(x_left, y_top, patch_width, patch_height), QPen(), QBrush(colors[n_cols * y + x]));

            if (!_model->getSelectedPatches()[y * n_cols + x]) {
                scene()->addLine(x_left, y_top, x_left + patch_width, y_top + patch_height, QPen(Qt::red, 3));
                scene()->addLine(x_left + patch_width, y_top, x_left, y_top + patch_height, QPen(Qt::red, 3));
                scene()->addRect(QRectF(x_left, y_top, patch_width, patch_height), QPen(Qt::red, 3));
            }
        }
    }

    fitInView(0, 0, _macbethWidth, _macbethHeight, Qt::KeepAspectRatio);
}
