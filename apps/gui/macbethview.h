#ifndef MACBETHVIEW_H
#define MACBETHVIEW_H

#include <QGraphicsView>
#include "macbethmodel.h"

class MacbethView: public QGraphicsView
{
  Q_OBJECT
public:
  MacbethView(QWidget *parent = nullptr);

public slots:
  void setModel(MacbethModel *model);
  void onMacbethChanged(const std::array<QColor, 24> &colors);

protected:
  void resizeEvent(QResizeEvent *event) override;


private:
  MacbethModel *_model;

  const float _macbethWidth  = 600;
  const float _macbethHeight = 400;
};

#endif   // MACBETHVIEW_H
