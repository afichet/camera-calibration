#include "macbethmodel.h"

MacbethModel::MacbethModel(QObject *parent): QObject(parent), _ready(false)
{
  for (size_t i = 0; i < _selectedPatches.size(); i++)
  {
    _selectedPatches[i] = true;
  }
}
