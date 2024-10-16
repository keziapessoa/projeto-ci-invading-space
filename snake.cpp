#include <opencv2/opencv.hpp>
#include <iostream>
#include <deque>
#include <cstdlib>
#include <ctime>

using namespace cv;
using namespace std;

class SnakeGame {
public:
    SnakeGame() : score(0), gameOver(false), gridSize(20), snakeDirection(3) { // Começa movendo para a direita
        srand(static_cast<unsigned>(time(0)));
        cv::namedWindow("Snake Game");
        if (!faceCascade.load("haarcascade_frontalface_default.xml")) {
            cerr << "Erro ao carregar o classificador de rosto!" << endl;
            exit(1);
        }
        spawnFood();
        initSnake();
    }

    void run() {
        Mat frame;
        VideoCapture cap(0); // Captura da câmera

        if (!cap.isOpened()) {
            cerr << "Erro ao abrir a câmera!" << endl;
            exit(1);
        }

        while (true) {
            cap >> frame;
            if (frame.empty()) break;

            flip(frame, frame, 1); // Inverte horizontalmente a imagem da câmera
            detectFace(frame); // Detecta o rosto e ajusta a direção da cobra

            if (gameOver) {
                putText(frame, "Game Over!", Point(150, 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);
                imshow("Snake Game", frame);
                waitKey(2000);
                break;
            }

            Mat gameFrame(400, 400, CV_8UC3, Scalar(0, 0, 0)); // Frame do jogo
            moveSnake();
            drawSnake(gameFrame);
            drawFood(gameFrame);
            showScore(gameFrame);

            imshow("Snake Game", gameFrame);
            if (waitKey(100) == 27) break; // Pressionar 'Esc' para sair
        }

        cap.release();
        cv::destroyAllWindows();
    }

private:
    int score;
    bool gameOver;
    const int gridSize;
    int snakeDirection; // 0: Cima, 1: Baixo, 2: Esquerda, 3: Direita
    deque<Point> snake;
    Point food;
    CascadeClassifier faceCascade;

    void initSnake() {
        snake.push_front(Point(10, 10)); // Cobrinha começa com um segmento
    }

    void spawnFood() {
        food.x = rand() % (400 / gridSize);
        food.y = rand() % (400 / gridSize);
    }

    void moveSnake() {
        Point head = snake.front();
        switch (snakeDirection) {
            case 0: head.y -= 1; break; // Cima
            case 1: head.y += 1; break; // Baixo
            case 2: head.x -= 1; break; // Esquerda
            case 3: head.x += 1; break; // Direita
        }

        if (head.x < 0 || head.x >= (400 / gridSize) || head.y < 0 || head.y >= (400 / gridSize)) {
            gameOver = true;
            return;
        }

        for (const auto& segment : snake) {
            if (segment == head) {
                gameOver = true;
                return;
            }
        }

        snake.push_front(head); // Adiciona a nova cabeça

        if (head == food) {
            score++;
            spawnFood(); // Gera novo alimento
        } else {
            snake.pop_back(); // Remove o último segmento se não comeu
        }
    }

    void drawSnake(Mat& frame) {
        for (const auto& segment : snake) {
            rectangle(frame, Rect(segment.x * gridSize, segment.y * gridSize, gridSize, gridSize), Scalar(0, 255, 0), -1);
        }
    }

    void drawFood(Mat& frame) {
        rectangle(frame, Rect(food.x * gridSize, food.y * gridSize, gridSize, gridSize), Scalar(0, 0, 255), -1);
    }

    void showScore(Mat& frame) {
        putText(frame, "Score: " + to_string(score), Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
    }

    void detectFace(Mat& frame) {
        vector<Rect> faces;
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        faceCascade.detectMultiScale(gray, faces, 1.1, 3, 0, Size(30, 30)); // Ajustar tamanho mínimo

        if (!faces.empty()) {
            Rect face = faces[0]; // Considera apenas o primeiro rosto detectado
            Point center(face.x + face.width / 2, face.y + face.height / 2);

            // Define a direção da cobrinha com base na posição do rosto
            if (center.y < frame.rows / 3) {
                snakeDirection = 0; // Cima
            } else if (center.y > 2 * frame.rows / 3) {
                snakeDirection = 1; // Baixo
            } else if (center.x < frame.cols / 3) {
                snakeDirection = 2; // Esquerda
            } else if (center.x > 2 * frame.cols / 3) {
                snakeDirection = 3; // Direita
            }
        }
    }
};

int main() {
    SnakeGame game;
    game.run();
    return 0;
}
