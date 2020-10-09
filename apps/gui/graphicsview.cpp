#include "graphicsview.h"

#include <QGraphicsPixmapItem>
#include <QPen>
#include <QPainter>
#include <QMouseEvent>

GraphicsView::GraphicsView(QWidget *parent): QGraphicsView(parent), _model(nullptr), _imageItem(nullptr)
{
  setScene(new QGraphicsScene);
  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

GraphicsView::~GraphicsView() {}

void GraphicsView::setModel(MacbethModel *model)
{
  _model = model;

  connect(_model, SIGNAL(imageChanged()), this, SLOT(onImageChanged()));
  connect(_model, SIGNAL(macbethChartChanged()), this, SLOT(onMacbethChartChanged()));
}

void GraphicsView::onImageChanged()
{
  if (_model == nullptr) return;

  if (_imageItem != nullptr)
  {
    scene()->removeItem(_imageItem);
    delete _imageItem;
    _imageItem = nullptr;
  }

  //scene()->clear();
  const QImage &image = _model->getLoadedImage();
  _imageItem          = scene()->addPixmap(QPixmap::fromImage(image));
  fitInView(image.rect(), Qt::KeepAspectRatio);

  onMacbethChartChanged();
}


void GraphicsView::onMacbethChartChanged()
{
  if (_model == nullptr) return;

  for (QGraphicsPolygonItem *item : _chartItems)
  {
    scene()->removeItem(item);
    delete item;
  }

  _chartItems.clear();

  const QPolygonF &         macbethOutline = _model->getMacbethOutline();
  const QVector<QPolygonF> &macbethPatches = _model->getMacbethPatches();

  QPen pen(Qt::green);
  pen.setWidth(3);

  _chartItems << scene()->addPolygon(macbethOutline, pen);

  for (const QPolygonF &patch : macbethPatches)
  {
    _chartItems << scene()->addPolygon(patch, pen);
  }

  if (_inSelection)
  {
    if (_selection != nullptr)
    {
      scene()->removeItem(_selection);
      delete _selection;
    }

    float r = 100.;
    pen.setColor(Qt::red);
    pen.setWidth(10.);

    _selection = scene()->addEllipse(
        macbethOutline[_selectedIdx].x() - r / 2.f,
        macbethOutline[_selectedIdx].y() - r / 2.f,
        r,
        r,
        pen);
  }
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
  if (_model == nullptr) return;
  _inSelection = true;

  const QPointF selection = mapToScene(event->pos());
  // Find the closest corner
  float distance = std::numeric_limits<float>::max();

  const QPolygonF &macbethOutline = _model->getMacbethOutline();

  for (int i = 0; i < macbethOutline.size(); i++)
  {
    const float currDistance = QLineF(selection, macbethOutline[i]).length();

    if (currDistance < distance)
    {
      distance     = currDistance;
      _selectedIdx = i;
    }
  }

  emit onMacbethChartChanged();
}


void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
  if (_model == nullptr) return;

  if (_inSelection)
  {
    _model->setOutlinePosition(_selectedIdx, mapToScene(event->pos()));
  }
}


void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
  if (_model == nullptr) return;

  if (_inSelection)
  {
    _model->setOutlinePosition(_selectedIdx, mapToScene(event->pos()));
    _inSelection = false;
  }

  emit onMacbethChartChanged();
}
