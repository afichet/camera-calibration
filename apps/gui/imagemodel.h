#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H

#include <QObject>
#include <QImage>
#include <QVector>
#include <QFutureWatcher>

class ImageModel: public QObject
{
  Q_OBJECT

public:
  ImageModel();
  virtual ~ImageModel();

  const QImage &            getLoadedImage() const { return _image; }
  const QPolygonF &         getMacbethOutline() const { return _macbethOutline; }
  const QVector<QPolygonF> &getMacbethPatches() const { return _macbethPatches; }
  const QVector<QPointF> &  getMacbethPatchesCenters() const { return _macbethPatchesCenters; }

  const std::array<float, 9> &getCorrectionMatrix() const { return _correctionMatrix; }

  void getAveragedPatches(std::vector<float> &values);

  bool isImageLoaded() const { return _isImageLoaded; }
  bool isMatrixLoaded() const { return _isMatrixLoaded; }
  bool isMatrixActive() const { return _isMatrixActive; }

public slots:
  void openFile(const QString &filename);
  void openImage(const QString &filename);
  void openCorrectionMatrix(const QString &filename);

  void setInnerMarginX(float position);
  void setInnerMarginY(float position);

  void setOutlinePosition(int index, QPointF position);

  void setExposure(double value);
  void setMatrix(const std::array<float, 9> matrix);
  void setMatrixActive(bool active);

  void savePatches(const QString &filename);
  void saveMatrix(const QString &filename);

signals:
  void macbethChartChanged();
  void imageChanged();
  void imageLoaded(int width, int height);
  void exposureChanged(double exposure);
  void loadFailed(QString message);
  void processProgress(int progress);
  void loadingMessage(QString const &message);
  void matrixLoaded(const std::array<float, 9> &matrix);
  void matrixActivationStateChanged(bool state);


protected:
  void recalculateCorrection(double exposure);

  void recalculateMacbethPatches();

private:
  float *              _pixelBuffer;
  std::vector<float>   _pixelCorrected;
  std::array<float, 9> _correctionMatrix;

  QImage _image;
  bool   _isImageLoaded;
  bool   _isMatrixLoaded;
  bool   _isMatrixActive;

  float _innerMarginX, _innerMarginY;

  QPolygonF          _macbethOutline;
  QVector<QPolygonF> _macbethPatches;
  QVector<QPointF>   _macbethPatchesCenters;

  double _exposure;

  QFutureWatcher<void> *_processWatcher;
};

#endif   // IMAGEMODEL_H
