#ifndef MACBETHMODEL_H
#define MACBETHMODEL_H

#include <QObject>
#include <QColor>
#include <array>

class MacbethModel: public QObject
{
  Q_OBJECT
public:
  explicit MacbethModel(QObject *parent = nullptr);

  bool ready() const { return _ready; }

  const std::array<QColor, 24> &getPatchesColors() const { return _tonemappedColors; }
  const std::array<float, 72> & getLinearColors() const { return _linearColors; }
  const std::array<bool, 24> &  getSelectedPatches() const { return _selectedPatches; }

signals:
  void macbethChanged(const std::array<QColor, 24> &tonemappedColors);

protected:
  std::array<float, 72>  _linearColors;
  std::array<QColor, 24> _tonemappedColors;
  std::array<bool, 24>   _selectedPatches;

  bool _ready;
};

#endif   // MACBETHMODEL_H
