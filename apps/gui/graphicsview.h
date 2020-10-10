#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

#include "macbethmodel.h"

class GraphicsView: public QGraphicsView
{
  Q_OBJECT
public:
  GraphicsView(QWidget *parent = nullptr);
  virtual ~GraphicsView();

public slots:
  void setModel(MacbethModel *model);

  void onImageChanged();
  void onMacbethChartChanged();

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  MacbethModel *       _model;
  QGraphicsPixmapItem *_imageItem;

  QVector<QGraphicsItem *> _chartItems;

  bool                  _inSelection;
  int                   _selectedIdx;
  QGraphicsEllipseItem *_selection;
};


#endif   // GRAPHICSVIEW_H
