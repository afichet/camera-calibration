#ifndef MACBETHREFERENCEMODEL_H
#define MACBETHREFERENCEMODEL_H

#include <macbethmodel.h>

class MacbethReferenceModel: public MacbethModel
{
    Q_OBJECT
  public:
    explicit MacbethReferenceModel(
      const float* illuminant_spd,   // 1nm spacing
      int          illuminant_first_wavelength_nm,
      size_t       illuminant_size,
      const float* cmf_x,   // 1nm spacing
      const float* cmf_y,   // 1nm spacing
      const float* cmf_z,   // 1nm spacing
      int          cmf_first_wavelength_nm,
      size_t       cmf_size,
      QObject*     parent = nullptr);

    void setIlluminant(
      const float* illuminant_spd,   // 1nm spacing
      int          illuminant_first_wavelength_nm,
      size_t       illuminant_size);

    void setCMFs(
      const float* cmf_x,   // 1nm spacing
      const float* cmf_y,   // 1nm spacing
      const float* cmf_z,   // 1nm spacing
      int          cmf_first_wavelength_nm,
      size_t       cmf_size);

  signals:

  protected:
    void updateColors();


  private:
    std::vector<float> _illuminantSPD;
    int                _illuminantFirstWavelength;

    std::vector<float> _cmfX;
    std::vector<float> _cmfY;
    std::vector<float> _cmfZ;
    int                _cmfFirstWavelength;
};

#endif   // MACBETHREFERENCEMODEL_H
