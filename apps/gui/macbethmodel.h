#ifndef MACBETHMODEL_H
#define MACBETHMODEL_H

#include <QObject>
#include <QImage>
#include <QVector>

class MacbethModel: public QObject
{
  Q_OBJECT

public:
  MacbethModel();
  virtual ~MacbethModel();

  const QImage &            getLoadedImage() const { return _image; }
  const QPolygonF &         getMacbethOutline() const { return _macbethOutline; }
  const QVector<QPolygonF> &getMacbethPatches() const { return _macbethPatches; }


public slots:
  void openFile(const QString &filename);

  void recalculateMacbethPatches();

  void setInnerMarginX(float position);
  void setInnerMarginY(float position);

  void setOutlinePosition(int index, QPointF position);

  void setExposure(double value);

signals:
  void macbethChartChanged();
  void imageChanged();

private:
  float *_pixelBuffer;
  QImage _image;

  float _innerMarginX, _innerMarginY;

  QPolygonF          _macbethOutline;
  QVector<QPolygonF> _macbethPatches;
};

#endif   // MACBETHMODEL_H
