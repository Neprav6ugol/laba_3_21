#include <QApplication>
#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <vector>
#include <QDebug>
#include <QVector>
#include <QSizeGrip>
#include <QSplitter>
#include "./ui_mainwindow.h"

using namespace std;

class CCircle : public QGraphicsEllipseItem {
public:
    CCircle(qreal posX, qreal posY, int radius)
        : QGraphicsEllipseItem(QRect(posX, posY, radius, radius), nullptr), selected(false) {
        setAcceptHoverEvents(true);
        this->radius = radius;
        centerX = posX;
        centerY = posY;
        color = QColor(
            QRandomGenerator::global()->bounded(240),
            QRandomGenerator::global()->bounded(256),
            QRandomGenerator::global()->bounded(256)
            );
        setBrush(QBrush(color));
        setPen(QPen(Qt::white, 2));
    }

    void setSelected(bool selected) {
        this->selected = selected;
        setBrush(selected ? QBrush(Qt::red) : QBrush(color));
    }

    bool isSelected() const {
        return selected;
    }

    bool isCursorFocused(const QPointF& positionCursor) const {
        return contains(mapFromScene(positionCursor));
    }

    qreal getX() const {
        return centerX;
    }

    qreal getY() const {
        return centerY;
    }

private:
    bool selected;
    int radius;
    QColor color;
    qreal centerX, centerY;
};

class CCircleStorage {
private:
    QVector<CCircle*> circles;
    int currentIndex = 0;

public:
    void add(CCircle* circle) {
        circles.append(circle);
    }

    void clear() {
        for (CCircle* circle : circles) {
            delete circle;
        }
        circles.clear();
    }

    CCircle* current() const {
        if (!isEol()) {
            return circles[currentIndex];
        }
        return nullptr;
    }

    bool isEol() const {
        return currentIndex >= circles.size();
    }

    void next() {
        if (!isEol()) currentIndex++;
    }

    void first() {
        currentIndex = 0;
    }

    CCircle* getShape(int index) {
        if (index >= 0 && index < circles.size()) {
            return circles[index];
        }
        return nullptr;
    }

    void deleteShape(int index) {
        if (index >= 0 && index < circles.size()) {
            delete circles[index];
            circles.remove(index);
        }
    }

    QVector<CCircle*> getSelectedCircles() const {
        QVector<CCircle*> selectedCircles;
        for (CCircle* circle : circles) {
            if (circle->isSelected()) {
                selectedCircles.append(circle);
            }
        }
        return selectedCircles;
    }

    void deleteSelectedCircles() {
        QVector<CCircle*> selected = getSelectedCircles();
        for (CCircle* circle : selected) {
            circles.removeAll(circle);
            circle->scene()->removeItem(circle);
            delete circle;
        }
    }
};

class CircleView : public QGraphicsView {
public:
    CircleView(QGraphicsScene *scene, CCircleStorage *storage, QWidget *parent = nullptr)
        : QGraphicsView(scene, parent), storage(storage) {
        setMouseTracking(true);
        setRenderHint(QPainter::Antialiasing);
        setBackgroundBrush(Qt::white);  // Фон теперь белый
        fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QGraphicsView::resizeEvent(event);
        fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    }

    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Delete) {
            storage->deleteSelectedCircles();
        }
        QGraphicsView::keyPressEvent(event);
    }

    void mousePressEvent(QMouseEvent *event) override {
        QPointF scenePos = mapToScene(event->pos());

        if (event->button() == Qt::RightButton) {
            addCircle(scenePos);
        }

        if (event->button() == Qt::LeftButton) {
            bool controlPressed = event->modifiers() & Qt::ControlModifier;


            if (!controlPressed) {
                deselectAll();
            }

            selectCircles(scenePos, false);
            lastMousePos = scenePos;
        }

        QGraphicsView::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (event->buttons() & Qt::LeftButton) {
            QPointF delta = mapToScene(event->pos()) - lastMousePos;
            moveSelectedCircles(delta);
            lastMousePos = mapToScene(event->pos());
        }
        QGraphicsView::mouseMoveEvent(event);
    }

private:
    void addCircle(const QPointF &pos) {
        CCircle* circle = new CCircle(pos.x() - 50, pos.y() - 50, 100);
        scene()->addItem(circle);
        storage->add(circle);
    }

    void selectCircles(const QPointF& scenePos, bool single) {
        QList<QGraphicsItem*> items = scene()->items(scenePos);
        for (QGraphicsItem* item : items) {
            CCircle* circle = dynamic_cast<CCircle*>(item);
            if (circle && circle->isCursorFocused(scenePos)) {
                circle->setSelected(true);
                if (single) return;
                for (auto other : storage->getSelectedCircles()) {
                    if (circle->collidesWithItem(other)) {
                        other->setSelected(true);
                    }
                }
            }
        }
    }

    void moveSelectedCircles(const QPointF& delta) {
        for (CCircle* circle : storage->getSelectedCircles()) {
            circle->moveBy(delta.x(), delta.y());
        }
    }

    void deselectAll() {
        for (CCircle* circle : storage->getSelectedCircles()) {
            circle->setSelected(false);
        }
    }

    QPointF lastMousePos;
    CCircleStorage* storage;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    Ui::MainWindow *ui;
    CCircleStorage storage;
    CircleView *view;

    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        ui = new Ui::MainWindow;
        ui->setupUi(this);

        QGraphicsScene *scene = new QGraphicsScene(this);
        scene->setSceneRect(0, 0, 800, 600);

        view = new CircleView(scene, &storage, this);

        QWidget *rightWidget = new QWidget(this);
        QWidget *bottomWidget = new QWidget(this);

        QSplitter *horizontalSplitter = new QSplitter(Qt::Horizontal, this);
        horizontalSplitter->addWidget(view);
        horizontalSplitter->addWidget(rightWidget);
        horizontalSplitter->setStretchFactor(0, 4);

        QSplitter *verticalSplitter = new QSplitter(Qt::Vertical, this);
        verticalSplitter->addWidget(horizontalSplitter);
        verticalSplitter->addWidget(bottomWidget);
        verticalSplitter->setStretchFactor(0, 5);

        setCentralWidget(verticalSplitter);
        setWindowTitle("Добавление и перемещение кругов");
        resize(1000, 800);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
