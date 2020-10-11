#ifndef MACBETHMODEL_H
#define MACBETHMODEL_H

#include <QObject>
#include <QImage>
#include <QVector>
#include <QFutureWatcher>

class MacbethModel: public QObject
{
  Q_OBJECT

public:
  MacbethModel();
  virtual ~MacbethModel();

  const QImage &            getLoadedImage() const { return _image; }
  const QPolygonF &         getMacbethOutline() const { return _macbethOutline; }
  const QVector<QPolygonF> &getMacbethPatches() const { return _macbethPatches; }
  const QVector<QPointF> &  getMacbethPatchesCenters() const { return _macbethPatchesCenters; }

  bool isImageLoaded() const { return _imageLoaded; }

public slots:
  void openFile(const QString &filename);

  void recalculateMacbethPatches();

  void setInnerMarginX(float position);
  void setInnerMarginY(float position);

  void setOutlinePosition(int index, QPointF position);

  void setExposure(double value);

  void savePatches(const QString &filename);

signals:
  void macbethChartChanged();
  void imageChanged();
  void imageLoaded(int width, int height);
  void exposureChanged(double exposure);
  void loadFailed(QString message);

private:
  float *_pixelBuffer;
  QImage _image;
  bool   _imageLoaded;

  float _innerMarginX, _innerMarginY;

  QPolygonF          _macbethOutline;
  QVector<QPolygonF> _macbethPatches;
  QVector<QPointF>   _macbethPatchesCenters;

  double _exposure;

  QFutureWatcher<void> *_processWatcher;
};

#endif   // MACBETHMODEL_H
