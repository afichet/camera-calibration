#include "fittingdialog.h"
#include "ui_fittingdialog.h"

#include <QTimer>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#include <radiometry.h>

extern "C"
{
#include <levmar.h>
#include <color-converter.h>
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
  });

  _processWatcher->setFuture(imageLoading);
}

void FittingDialog::on_colorMatchingFunctions_currentIndexChanged(const QString &arg1) {}

void FittingDialog::on_illuminant_currentIndexChanged(const QString &arg1) {}

void FittingDialog::on_applyMatrix_toggled(bool checked)
{
  _measured.setMatrixActive(checked);
}
