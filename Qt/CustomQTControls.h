/*
 * 2022-12-22 daegyumin@gmail.com
 *
 * Application 에서 사용되는 모든 Custon QT Control 들이 정의된 파일
 *
 */

#ifndef CUSTOM_QT_CONTROLS_H
#define CUSTOM_QT_CONTROLS_H
#include <qdialog.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qshortcut.h>
#include <qthread.h>
#include <qwidget.h>

#include <QBoxLayout>
#include <QEvent>

/*----------------------------------------------------------------------------------------------------------------------------*/

class ProgressDialog : public QDialog {
  Q_OBJECT

  class ProgressThread : public QThread {
   public:
    ProgressThread(ProgressDialog* pProgDlg);
    virtual void run();
  };

 public:
  ProgressDialog(const QString& title, const QString& text,
                 QWidget* parent = 0);

  void SetWork(QObject* target, void* targetParam,
               void (*targetFunc)(void*, void*, bool*, int*));

  void setVisibleCancelButton(bool bVisible);

  QString text() const;

  int GetErrorCode() const { return _nErrorCode; }

  void SetUseProgressValue(bool bFlag);

 public slots:

  void setTitle(const QString& title);
  void setText(const QString& text);
  void setProgress(double fValue);

 protected:
  virtual void showEvent(QShowEvent* event);
  virtual void keyPressEvent(QKeyEvent* event);

 Q_SIGNALS:
  void SIGNAL_CANCEL();

 protected slots:

  virtual void onCancel();
  void OnProgressFinished();

 private:
  QObject* _target;
  void* _targetParam;
  void (*_targetFunc)(void*, void*, bool*, int*);

  bool _bCancel;
  int _nErrorCode;
  ProgressThread* m_pThread;

  QLabel m_textLabel;
  QProgressBar m_progressbar;
  QPushButton m_cancelButton;

  void onCreateView();
};

#endif  // CUSTOM_QT_CONTROLS_H