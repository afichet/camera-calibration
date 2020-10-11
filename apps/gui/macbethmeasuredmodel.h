#ifndef MACBETHMEASUREDMODEL_H
#define MACBETHMEASUREDMODEL_H

#include <macbethmodel.h>

class MacbethMeasuredModel: public MacbethModel
{
  Q_OBJECT
public:
  explicit MacbethMeasuredModel(QObject *parent = nullptr);

  void setPatchesValues(const std::vector<float> &values);

  const std::array<float, 9> &getCorrectionMatrix() const { return _correctionMatrix; }

  bool isMatrixActive() const { return _isMatrixActive; }

public slots:
  void setMatrix(const std::array<float, 9> matrix);
  void setMatrixActive(bool active);

signals:
  void matrixChanged(const std::array<float, 9> &matrix);
  void matrixActivationStateChanged(bool state);

protected:
  void updateColors();

private:
  std::array<float, 9> _correctionMatrix;
  bool                 _isMatrixActive;
};

#endif   // MACBETHMEASUREDMODEL_H
