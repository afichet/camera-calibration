#include "fittingdialog.h"
#include "ui_fittingdialog.h"

#include <QTimer>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QFileDialog>

#include <radiometry.h>

extern "C"
{
#include <levmar.h>
#include <color-converter.h>
#include <spectrum-converter.h>
#include <io.h>
}

FittingDialog::FittingDialog(ImageModel *model, QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::FittingDialog)
  , _measured(this)
  , _reference(
        D_65_SPD,
        D_65_FIRST_WAVELENGTH,
        D_65_ARRAY_SIZE,
        SENSITIVITY_CIE_1931_2DEG_X,
        SENSITIVITY_CIE_1931_2DEG_Y,
        SENSITIVITY_CIE_1931_2DEG_Z,
        SENSITIVITY_CIE_1931_2DEG_FIRST_WAVELENGTH,
        SENSITIVITY_CIE_1931_2DEG_SIZE,
        this)
  , _image(model)
  , _processWatcher(new QFutureWatcher<void>(this))
{
  ui->setupUi(this);

  ui->reference->setModel(&_reference);
  ui->measured->setModel(&_measured);
}

FittingDialog::~FittingDialog()
{
  delete ui;
}

void FittingDialog::showEvent(QShowEvent *event)
{
  QDialog::showEvent(event);
  QTimer::singleShot(0, this, SLOT(initModels()));
}

void FittingDialog::measure(
    float *pParameters,
    float *pMeasurements,
    int    n_parameters,
    int    n_measurements,
    void * pUserParam)
{
  (void)n_parameters;
  (void)n_measurements;

  const fit_params *info = (fit_params *)pUserParam;

  float tristim_mea_corrected[3];
  float lab_ref[3];
  float lab_mea[3];

  // For each patch, we try to minimize the Delta_E_2000 between reference and measurement
  for (size_t patch_idx = 0; patch_idx < info->n_patches; patch_idx++)
  {
    const float *tristim_ref = &(info->reference_patches[3 * patch_idx]);
    const float *tristim_mea = &(info->measured_patches[3 * patch_idx]);

    // Apply the correction matrix which is optimized by levmar
    matmul(pParameters, tristim_mea, tristim_mea_corrected);

    // Transform colorspaces to Lab*
    XYZ_to_Lab(tristim_ref, lab_ref);
    XYZ_to_Lab(tristim_mea_corrected, lab_mea);

    // The error is the Delta_E_2000 between reference and corrected Lab* values
    pMeasurements[patch_idx] = deltaE_2000(lab_ref, lab_mea);
  }
}

void FittingDialog::fit()
{
  const size_t size   = 6 * 4;
  _fitMatrix          = {1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f};
  fit_params u_params = {size, &_reference.getLinearColors()[0], &_measured.getLinearColors()[0]};
  slevmar_dif(FittingDialog::measure, &_fitMatrix[0], NULL, 9, size, 1000, NULL, NULL, NULL, NULL, &u_params);
  _measured.setMatrix(_fitMatrix);
}

void FittingDialog::initModels()
{
  QFuture<void> imageLoading = QtConcurrent::run([=]() {
    std::vector<float> measuredValues;
    _image->getAveragedPatches(measuredValues);
    _measured.setPatchesValues(measuredValues);
    fit();
    ui->applyMatrix->setEnabled(true);
    ui->illuminant->setEnabled(true);
    ui->colorMatchingFunctions->setEnabled(true);
    ui->apply->setEnabled(true);
  });

  _processWatcher->setFuture(imageLoading);
}


void FittingDialog::on_applyMatrix_toggled(bool checked)
{
  _measured.setMatrixActive(checked);
}


void FittingDialog::on_colorMatchingFunctions_currentIndexChanged(int index)
{
  if (index == 0)
  {
    _reference.setCMFs(
        SENSITIVITY_CIE_1931_2DEG_X,
        SENSITIVITY_CIE_1931_2DEG_Y,
        SENSITIVITY_CIE_1931_2DEG_Z,
        SENSITIVITY_CIE_1931_2DEG_FIRST_WAVELENGTH,
        SENSITIVITY_CIE_1931_2DEG_SIZE);
  }
  else if (index == 1)
  {
    _reference.setCMFs(
        SENSITIVITY_CIE_1964_10DEG_X,
        SENSITIVITY_CIE_1964_10DEG_Y,
        SENSITIVITY_CIE_1964_10DEG_Z,
        SENSITIVITY_CIE_1964_10DEG_FIRST_WAVELENGTH,
        SENSITIVITY_CIE_1964_10DEG_SIZE);
  }
  else if (index == 2)
  {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("Spectrum (*.csv)"));

    if (filename.size() != 0)
    {
      int *  wavelengths_cmfs_raw = nullptr;
      float *values_cmfs_x_raw    = nullptr;
      float *values_cmfs_y_raw    = nullptr;
      float *values_cmfs_z_raw    = nullptr;
      size_t size_cmfs_raw        = 0;
      float *values_cmfs_x        = nullptr;
      float *values_cmfs_y        = nullptr;
      float *values_cmfs_z        = nullptr;
      size_t size_cmfs            = 0;

      int err = read_cmfs(
          filename.toStdString().c_str(),
          &wavelengths_cmfs_raw,
          &values_cmfs_x_raw,
          &values_cmfs_y_raw,
          &values_cmfs_z_raw,
          &size_cmfs_raw);

      if (err != 0)
      {
        // TODO
        fprintf(stderr, "Cannot open CMFs file\n");
      }
      else
      {
        spectrum_oversample(wavelengths_cmfs_raw, values_cmfs_x_raw, size_cmfs_raw, &values_cmfs_x, &size_cmfs);
        spectrum_oversample(wavelengths_cmfs_raw, values_cmfs_y_raw, size_cmfs_raw, &values_cmfs_y, &size_cmfs);
        spectrum_oversample(wavelengths_cmfs_raw, values_cmfs_z_raw, size_cmfs_raw, &values_cmfs_z, &size_cmfs);

        cmf_data d;
        d.firstWavelength = wavelengths_cmfs_raw[0];
        d.x.resize(size_cmfs);
        d.y.resize(size_cmfs);
        d.z.resize(size_cmfs);

        memcpy(d.x.data(), values_cmfs_x, size_cmfs * sizeof(float));
        memcpy(d.y.data(), values_cmfs_y, size_cmfs * sizeof(float));
        memcpy(d.z.data(), values_cmfs_z, size_cmfs * sizeof(float));

        _userCMFs.push_back(d);

        delete[] wavelengths_cmfs_raw;
        delete[] values_cmfs_x_raw;
        delete[] values_cmfs_y_raw;
        delete[] values_cmfs_z_raw;
        delete[] values_cmfs_x;
        delete[] values_cmfs_y;
        delete[] values_cmfs_z;

        const QFileInfo fileInfo(filename);

        const size_t insertIndex = ui->colorMatchingFunctions->count();
        ui->colorMatchingFunctions->insertItem(insertIndex, fileInfo.fileName());
        ui->colorMatchingFunctions->setCurrentIndex(insertIndex);
      }
    }
  }
  else
  {
    _reference.setCMFs(
        _userCMFs[index - 3].x.data(),
        _userCMFs[index - 3].y.data(),
        _userCMFs[index - 3].z.data(),
        _userCMFs[index - 3].firstWavelength,
        _userCMFs[index - 3].x.size());
  }

  fit();
}


void FittingDialog::on_illuminant_currentIndexChanged(int index)
{
  if (index == 0)
  {
    _reference.setIlluminant(D_65_SPD, D_65_FIRST_WAVELENGTH, D_65_ARRAY_SIZE);
  }
  else if (index == 1)
  {
    _reference.setIlluminant(D_50_SPD, D_50_FIRST_WAVELENGTH, D_50_ARRAY_SIZE);
  }
  else if (index == 2)
  {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("Spectrum (*.csv *.spd)"));

    if (filename.size() != 0)
    {
      int *  wavelengths_raw = nullptr;
      float *spd_raw         = nullptr;
      float *spd             = nullptr;
      size_t size_raw, size;

      int err = read_spd(filename.toStdString().c_str(), &wavelengths_raw, &spd_raw, &size_raw);

      if (err != 0)
      {
        // TODO
        fprintf(stderr, "Cannot open illuminant spd file\n");
      }
      else
      {
        spectrum_oversample(wavelengths_raw, spd_raw, size_raw, &spd, &size);

        illuminant_data d;
        d.firstWavelength = wavelengths_raw[0];
        d.illuminantSPD.resize(size);

        memcpy(d.illuminantSPD.data(), spd, size * sizeof(float));
        _userIlluminants.push_back(d);

        delete[] wavelengths_raw;
        delete[] spd_raw;
        delete[] spd;

        const QFileInfo fileInfo(filename);

        const size_t insertIndex = ui->illuminant->count();
        ui->illuminant->insertItem(insertIndex, fileInfo.fileName());
        ui->illuminant->setCurrentIndex(insertIndex);
      }
    }
  }
  else
  {
    _reference.setIlluminant(
        _userIlluminants[index - 3].illuminantSPD.data(),
        _userIlluminants[index - 3].firstWavelength,
        _userIlluminants[index - 3].illuminantSPD.size());
  }

  fit();
}
