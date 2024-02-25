
#include "CustomQTControls.h"

#include <qevent.h>

using namespace std;

ProgressDialog::ProgressThread::ProgressThread(ProgressDialog* pProgDlg)
  : QThread(pProgDlg) {}

void ProgressDialog::ProgressThread::run() {
  ProgressDialog* pProgDlg = (ProgressDialog*)parent();

  if (pProgDlg && pProgDlg->_targetFunc) {
    pProgDlg->_targetFunc(pProgDlg->_target, pProgDlg->_targetParam,
                          &pProgDlg->_bCancel, &pProgDlg->_nErrorCode);
  }
}

ProgressDialog::ProgressDialog(const QString& title, const QString& text,
                               QWidget* parent)
  : QDialog(parent),
    _bCancel(false),
    _nErrorCode(0),
    _target(nullptr),
    _targetParam(nullptr),
    _targetFunc(nullptr),
    m_pThread(nullptr) {
  Qt::WindowFlags wndFlags = windowFlags() | Qt::Dialog |
                             Qt::CustomizeWindowHint |
                             Qt::MSWindowsFixedSizeDialogHint;
  wndFlags &= ~(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);
  setWindowFlags(wndFlags);

  setTitle(title);
  setText(text);

  connect(&m_cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));

  onCreateView();
}

void ProgressDialog::SetWork(QObject* target, void* targetParam,
                             void (*targetFunc)(void*, void*, bool*, int*)) {
  _target = target;
  _targetParam = targetParam;
  _targetFunc = targetFunc;
}

void ProgressDialog::setVisibleCancelButton(bool bVisible) {
  m_cancelButton.setVisible(bVisible);
}

QString ProgressDialog::text() const { return m_textLabel.text(); }

void ProgressDialog::SetUseProgressValue(bool bFlag) {
  if (bFlag) {
    m_progressbar.setRange(0, 100);
  } else {
    m_progressbar.setRange(0, 0);
  }
}

void ProgressDialog::setTitle(const QString& title) { setWindowTitle(title); }

void ProgressDialog::setText(const QString& text) { m_textLabel.setText(text); }

void ProgressDialog::setProgress(double fValue) {
  const auto value =
    (int)((double)(m_progressbar.maximum() - m_progressbar.minimum()) * fValue);

  m_progressbar.setValue(value);
}

void ProgressDialog::onCancel() {
  _bCancel = true;

  emit SIGNAL_CANCEL();
}

void ProgressDialog::OnProgressFinished() { done(1); }

void ProgressDialog::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  auto parWidget = this->parentWidget();
  if (nullptr != parWidget) {
    const auto szWnd = this->size();
    const auto cen = parWidget->geometry().center();

    this->move(cen.x() - szWnd.width() / 2, cen.y() - szWnd.height() / 2);
  }

  if (nullptr == m_pThread) {
    m_pThread = new ProgressThread(this);
    connect(m_pThread, SIGNAL(finished()), this, SLOT(OnProgressFinished()));
    m_pThread->start(QThread::NormalPriority);
  }
}

void ProgressDialog::keyPressEvent(QKeyEvent* event) {
  if (event->key() != Qt::Key_Escape)
    QDialog::keyPressEvent(event);
}

void ProgressDialog::onCreateView() {
  QVBoxLayout* vLayout = new QVBoxLayout;
  vLayout->setSpacing(10);
  vLayout->setContentsMargins(0, 0, 0, 0);
  {
    m_textLabel.setAlignment(Qt::AlignCenter);
    vLayout->addWidget(&m_textLabel);
    m_progressbar.setRange(0, 0);
    vLayout->addWidget(&m_progressbar);

    m_cancelButton.setText("Cancel");
    vLayout->addWidget(&m_cancelButton, 0, Qt::AlignCenter);

    setLayout(vLayout);
  }
}
