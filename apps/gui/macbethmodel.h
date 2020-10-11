#ifndef MACBETHMODEL_H
#define MACBETHMODEL_H

#include <QObject>
#include <QColor>

class MacbethModel: public QObject
{
  Q_OBJECT
public:
  explicit MacbethModel(QObject *parent = nullptr);

  bool ready() const { return _ready; }

  const std::array<QColor, 24> &getPatchesColors() const { return _tonemappedColors; }
  const std::array<float, 72> & getLinearColors() const { return _linearColors; }
signals:
  void macbethChanged(const std::array<QColor, 24> &tonemappedColors);

protected:
  std::array<float, 72>  _linearColors;
  std::array<QColor, 24> _tonemappedColors;
  bool                   _ready;
};

#endif   // MACBETHMODEL_H
