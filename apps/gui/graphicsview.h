#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

#include "imagemodel.h"

class GraphicsView: public QGraphicsView
{
  Q_OBJECT
public:
  GraphicsView(QWidget *parent = nullptr);
  virtual ~GraphicsView();

public slots:
  void setModel(ImageModel *model);

  void onImageLoaded(int width, int height);
  void onImageChanged();
  void onMacbethChartChanged();
  void setShowPatchNumbers(bool show);
  void setZoomLevel(float zoom);
  void zoomIn();
  void zoomOut();

protected:
  //  void wheelEvent( QWheelEvent * event ) override;
  void resizeEvent(QResizeEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

  void dropEvent(QDropEvent *ev) override;
  void dragEnterEvent(QDragEnterEvent *ev) override;

private:
  ImageModel *         _model;
  QGraphicsPixmapItem *_imageItem;

  QVector<QGraphicsItem *> _chartItems;

  bool                  _inSelection;
  int                   _selectedIdx;
  QGraphicsEllipseItem *_selection;

  QPoint _startDrag;

  bool _showPatchNumbers;

  float _zoomLevel;
};


#endif   // GRAPHICSVIEW_H
