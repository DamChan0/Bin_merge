#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "CustomQTControls.h"

std::vector<QString> filePaths;
std::vector<std::string> fileVector;
int fileNum;
uint32_t option_result = 1;
uint32_t splitStart = 0;
uint32_t splitEnd;
template <typename... Args>
void MainWindow::QprintDone(const QString& message, Args&&... args) {
  QColor color("#080");

  QTextCharFormat format;
  format.setFontFamily("Arial");
  format.setFontPointSize(12);
  format.setFontWeight(QFont::Bold);
  format.setForeground(color);

  QTextCursor cursor(ui->consolPrint->textCursor());
  cursor.insertText(message, format);
  cursor.insertText("\n", format);
}

template <typename... Args>
void MainWindow::QprintErr(const QString& message, Args&&... args) {
  QColor color("#f11");
  // QTextCharFormat을 생성하여 스타일을 설정
  QTextCharFormat format;
  format.setFontFamily("Arial");
  format.setFontPointSize(30);
  format.setFontWeight(QFont::Bold);
  format.setForeground(color);

  QTextCursor cursor(ui->consolPrint->textCursor());
  cursor.insertText(message, format);
  cursor.insertText("\n", format);
}

template <typename... Args>
void MainWindow::QprintDebug(const QString& message, const Args&... args) {
  QColor color("#002");

  QTextCharFormat format;
  format.setFontFamily("Arial");
  format.setFontPointSize(18);
  format.setForeground(color);

  QTextCursor cursor(ui->consolPrint->textCursor());
  cursor.insertText(message + " ", format);
  ((cursor.insertText(QString::fromStdString(std::to_string(args)) + " ",
                      format)),
   ...);
  cursor.insertText("\n", format);
}

int MainWindow::launch() {
  if (filePaths.size() == 0) {
    return 0;
  }

  splitStart = ui->spinBoxMin->value();
  splitEnd = ui->spinBoxMax->value();

  fileVector.clear();
  for (int i = 0; i < fileNum; i++) {
    std::cout << "filePath: " << filePaths[i].toStdString()
              << "sz: " << filePaths.size() << std::endl;
    const QString& filePath = filePaths[i];

    fileVector.push_back(filePath.toStdString());
    std::cout << fileVector.back() << std::endl;
  }

  struct stWorker_FileIO {
    uint32_t fileNum;
    uint32_t option_result;
    uint32_t splitStart;
    uint32_t splitEnd;
    std::vector<std::string> fileVector;

    stWorker_FileIO() : prog_cb(nullptr) {}

    void (*prog_cb)(double);
  };

  stWorker_FileIO thData;
  thData.fileNum = fileNum;
  thData.fileVector = fileVector;
  thData.option_result = option_result;
  thData.splitEnd = splitEnd;
  thData.splitStart = splitStart;
  thData.prog_cb = [](double fValue) {};

  ProgressDialog thDlg(("MergeLidar"), ("병합 진행중"), this);

  thDlg.setVisibleCancelButton(false);
  thDlg.SetUseProgressValue(false);

  QPushButton exit;

  thDlg.SetWork(
    this, (void*)&thData,
    [](void* pThis, void* pParam, bool* pbCancel, int* pnErrorCode) {
      auto param = (stWorker_FileIO*)pParam;
      if (nullptr == param)
        return;

      BinMergeMain::Merge mergeMain;
      mergeMain.Merge::BinMerge(param->fileNum, param->fileVector,
                                param->option_result, param->splitStart,
                                param->splitEnd);
    });
  thDlg.exec();

  QprintDone("Merge && Split is DONE");

  return 0;
}

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  splitStart = 0;
  splitEnd = 0;

  connect(ui->fileNumComboBox, QOverload<int>::of(&QComboBox::activated),
          [&](int index) {
            fileNum = index + 1;
            QLayout* existingLayout = ui->fileList_scroll->layout();
            if (existingLayout) {
              QLayoutItem* child;
              while ((child = existingLayout->takeAt(0)) != nullptr) {
                delete child->widget();
                delete child;
              }
              delete existingLayout;
            }
            QWidget* fileSelecterWidget = new QWidget;

            QVBoxLayout* layout = new QVBoxLayout(fileSelecterWidget);
            layout->setSpacing(10);
            for (int i = 0; i < index + 1; ++i) {
              QWidget* fileSelecter = new QWidget(this);

              QPushButton* fileSelectButton =
                new QPushButton("파일 선택" + QString::number(i + 1));
              QTextEdit* filePathEdit = new QTextEdit();

              filePathEdit->toPlainText();
              filePathEdit->setMinimumHeight(100);
              fileSelectButton->setMinimumHeight(30);
              connect(fileSelectButton, &QPushButton::clicked,
                      [this, filePathEdit]() {
                        QString filePath =
                          QFileDialog::getOpenFileName(this, tr("파일 선택"));
                        if (!filePath.isEmpty()) {
                          filePaths.push_back(filePath);
                          qDebug() << "선택된 파일 경로:" << filePath;
                          filePathEdit->setPlainText(filePath);
                        }
                      });
              fileSelectButton->setStyleSheet("color: red;");
              layout->addWidget(fileSelectButton);
              layout->addWidget(filePathEdit);
            }

            ui->fileList_scroll->setWidget(fileSelecterWidget);
          });

  connect(ui->listCleanButton, &QPushButton::clicked, [&]() {
    QWidget* fileListWidget = ui->fileList_scroll->widget();
    QLayout* layout = fileListWidget->layout();
    if (layout) {
      int count = layout->count();
      for (int i = 0; i < count; ++i) {
        QLayoutItem* item = layout->itemAt(i);
        if (item->widget()) {
          QTextEdit* textEdit = qobject_cast<QTextEdit*>(item->widget());
          if (textEdit) {
            textEdit->clear();
          }
        }
      }
      filePaths.clear();
    }
    splitEnd = 0;
    splitStart = 0;
    ui->spinBoxMax->setValue(0);
    ui->spinBoxMin->setValue(0);
  });
  connect(ui->optionComboBox,
          QOverload<const QString&>::of(&QComboBox::activated),
          [&](const QString& text) {
            splitStart = ui->spinBoxMin->value();
            splitEnd = ui->spinBoxMax->value();

            if (text == "Force Merge") {
              QprintDebug("Force");
              option_result = 1;
            } else if (text == "Time Merge") {
              QprintDebug("TimeCompare");
              option_result = 2;
            } else if (text == "Split") {
              QprintDebug("Split");
              option_result = 3;
            }
            std::cout << "merge mode: " << option_result << std::endl;
          });

  connect(ui->launchButton, &QPushButton::clicked, this, &MainWindow::launch);

  QString imagePath =
    QCoreApplication::applicationDirPath() + "/../Qt/image/vueron.png";
  QPixmap pixmap(imagePath);
  ui->image_label->setPixmap(pixmap);
}

MainWindow::~MainWindow() { delete ui; }
