#include "graphicsview.h"
#include "graphicsscene.h"

#include <QGraphicsPixmapItem>
#include <QPen>
#include <QPainter>
#include <QMouseEvent>
#include <QUrl>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QGuiApplication>
#include <QScrollBar>

GraphicsView::GraphicsView(QWidget *parent)
  : QGraphicsView(parent)
  , _model(nullptr)
  , _imageItem(nullptr)
  , _inSelection(false)
  , _selection(nullptr)
  , _showPatchNumbers(false)
  , _zoomLevel(1.f)
{
  setScene(new GraphicsScene);
  //  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  setMouseTracking(true);
  setAcceptDrops(true);
}


GraphicsView::~GraphicsView() {}


void GraphicsView::setModel(ImageModel *model)
{
  _model = model;

  connect(_model, SIGNAL(imageChanged()), this, SLOT(onImageChanged()));
  connect(_model, SIGNAL(imageLoaded(int, int)), this, SLOT(onImageLoaded(int, int)));
  connect(_model, SIGNAL(macbethChartChanged()), this, SLOT(onMacbethChartChanged()));
}


void GraphicsView::onImageLoaded(int width, int height)
{
  _zoomLevel = 1.f;
  fitInView(0, 0, width, height, Qt::KeepAspectRatio);
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

  onMacbethChartChanged();
}


void GraphicsView::onMacbethChartChanged()
{
  if (_model == nullptr || !_model->isImageLoaded()) return;

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

  for (const QPolygonF &patch : macbethPatches)
  {
    _chartItems << scene()->addPolygon(patch, pen);
  }

  if (_showPatchNumbers)
  {
    const QVector<QPointF> &patchCenters = _model->getMacbethPatchesCenters();

    for (int i = 0; i < patchCenters.size(); i++)
    {
      QGraphicsTextItem *text = scene()->addText(QString::number(i + 1));
      text->setDefaultTextColor(Qt::red);
      text->setPos(patchCenters[i]);
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


void GraphicsView::setZoomLevel(float zoom)
{
  _zoomLevel = std::max(0.01f, zoom);
  resetTransform();
  scale(_zoomLevel, _zoomLevel);
}


void GraphicsView::zoomIn()
{
  setZoomLevel(_zoomLevel * 1.2);
}


void GraphicsView::zoomOut()
{
  setZoomLevel(_zoomLevel / 1.2);
}


void GraphicsView::wheelEvent(QWheelEvent *event)
{
  if ((event->modifiers() & Qt::ControlModifier) != 0U)
  {
    QGraphicsView::wheelEvent(event);
  }
  else
  {
    const QPoint delta = event->angleDelta();

    if (delta.y() != 0)
    {
      const double zoom_factor = 1.2 * float(std::abs(delta.y())) / 120.F;

      if (delta.y() > 0)
      {
        setZoomLevel(_zoomLevel * zoom_factor);
      }
      else
      {
        setZoomLevel(_zoomLevel / zoom_factor);
      }
    }
  }
}


void GraphicsView::resizeEvent(QResizeEvent *e)
{
  QGraphicsView::resizeEvent(e);
  if (_model == nullptr || !_model->isImageLoaded()) return;

  fitInView(0, 0, _model->getLoadedImage().width(), _model->getLoadedImage().height(), Qt::KeepAspectRatio);
}


void GraphicsView::mousePressEvent(QMouseEvent *event)
{
  if (_model == nullptr || !_model->isImageLoaded()) return;

  if (event->button() == Qt::LeftButton)
  {
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

    _model->setOutlinePosition(_selectedIdx, selection);
  }
  else if (
      (event->button() == Qt::MidButton)
      || (event->button() == Qt::LeftButton && QGuiApplication::keyboardModifiers() == Qt::ControlModifier))
  {
    QGraphicsView::mousePressEvent(event);
    setCursor(Qt::ClosedHandCursor);
    _startDrag = event->pos();
    return;
  }
}


void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
  if (_model == nullptr || !_model->isImageLoaded()) return;

  if (_inSelection)
  {
    _model->setOutlinePosition(_selectedIdx, mapToScene(event->pos()));
  }
  else if (
      ((event->buttons() & Qt::MidButton) != 0U)
      || (((event->buttons() & Qt::LeftButton) != 0U) && QGuiApplication::keyboardModifiers() == Qt::ControlModifier))
  {
    QScrollBar *        hBar  = horizontalScrollBar();
    QScrollBar *        vBar  = verticalScrollBar();
    QPoint              delta = event->pos() - _startDrag;
    std::pair<int, int> bar_values;
    bar_values.first  = hBar->value() + (isRightToLeft() ? delta.x() : -delta.x());
    bar_values.second = vBar->value() - delta.y();
    hBar->setValue(bar_values.first);
    vBar->setValue(bar_values.second);
    _startDrag = event->pos();
  }
}


void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
  if (_model == nullptr || !_model->isImageLoaded()) return;

  if (_inSelection)
  {
    _model->setOutlinePosition(_selectedIdx, mapToScene(event->pos()));
    _inSelection = false;
  }

  setCursor(Qt::ArrowCursor);

  emit onMacbethChartChanged();
}


void GraphicsView::dropEvent(QDropEvent *ev)
{
  if (_model == nullptr) return;

  QList<QUrl> urls = ev->mimeData()->urls();

  if (!urls.empty())
  {
    QString fileName = urls[0].toString();
    QString startFileTypeString =
#ifdef _WIN32
        "file:///";
#else
        "file://";
#endif

    if (fileName.startsWith(startFileTypeString))
    {
      fileName = fileName.remove(0, startFileTypeString.length());
      _model->openFile(fileName);
    }
  }
}


void GraphicsView::dragEnterEvent(QDragEnterEvent *ev)
{
  ev->acceptProposedAction();
}
