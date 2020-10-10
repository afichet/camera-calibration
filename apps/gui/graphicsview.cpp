#include "graphicsview.h"

#include <QGraphicsPixmapItem>
#include <QPen>
#include <QPainter>
#include <QMouseEvent>

GraphicsView::GraphicsView(QWidget *parent)
  : QGraphicsView(parent)
  , _model(nullptr)
  , _imageItem(nullptr)
  , _inSelection(false)
  , _selection(nullptr)
  , _showPatchNumbers(false)
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

  const float ratio = _model->getLoadedImage().width() / 500;

  for (QGraphicsItem *item : _chartItems)
  {
    scene()->removeItem(item);
    delete item;
  }

  _chartItems.clear();

  const QPolygonF &         macbethOutline = _model->getMacbethOutline();
  const QVector<QPolygonF> &macbethPatches = _model->getMacbethPatches();

  QPen pen(Qt::green);
  pen.setWidth(2. * ratio);

  _chartItems << scene()->addPolygon(macbethOutline, pen);

  pen.setWidth(1. * ratio);

  for (int i = 0; i < macbethPatches.size(); i++)
  {
    const QPolygonF &patch = macbethPatches[i];
    _chartItems << scene()->addPolygon(patch, pen);

    if (_showPatchNumbers)
    {
      QGraphicsTextItem *text = scene()->addText(QString::number(i + 1));
      text->setDefaultTextColor(Qt::red);
      text->setPos((patch[0].x() + patch[2].x()) / 2.f, (patch[0].y() + patch[2].y()) / 2.f);
      text->setScale(ratio);
      _chartItems << text;
    }
  }

  if (_selection != nullptr)
  {
    scene()->removeItem(_selection);
    delete _selection;
    _selection = nullptr;
  }

  float r = 10. * ratio;

  for (int i = 0; i < macbethOutline.size(); i++)
  {
    if (_inSelection && i == _selectedIdx)
    {
      r = 20. * ratio;
      pen.setColor(Qt::yellow);
      pen.setWidth(4. * ratio);
    }
    else
    {
      r = 10. * ratio;
      pen.setColor(Qt::red);
      pen.setWidth(4. * ratio);
    }

    _chartItems << scene()->addEllipse(macbethOutline[i].x() - r / 2.f, macbethOutline[i].y() - r / 2.f, r, r, pen);
  }
}

void GraphicsView::setShowPatchNumbers(bool show)
{
  _showPatchNumbers = show;
  emit onMacbethChartChanged();
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
