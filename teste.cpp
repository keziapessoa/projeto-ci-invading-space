#include <opencv2/opencv.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>

using namespace cv;
using namespace std;

void drawImage(Mat frame, Mat img, int xPos, int yPos) {
    if (yPos + img.rows >= frame.rows || xPos + img.cols >= frame.cols || yPos < 0 || xPos < 0)
        return;

    Mat mask;
    vector<Mat> layers;

    split(img, layers);
    if (layers.size() == 4) {
        Mat rgb[3] = { layers[0], layers[1], layers[2] };
        mask = layers[3];
        merge(rgb, 3, img);
        img.copyTo(frame.rowRange(yPos, yPos + img.rows).colRange(xPos, xPos + img.cols), mask);
    } else {
        img.copyTo(frame.rowRange(yPos, yPos + img.rows).colRange(xPos, xPos + img.cols));
    }
}

void drawNave(Mat& background, Mat& nave, int x, int y) {
    drawImage(background, nave, x, y);
}

void drawShot(Mat& background, Mat& shot, int x, int y) {
    drawImage(background, shot, x, y);
}

void drawTarget(Mat& background, Mat& target, int x, int y) {
    drawImage(background, target, x, y);
}

void drawScore(Mat& background, Ptr<freetype::FreeType2>& ft2, int score, Scalar color) {
    int fontScale = 30;
    int baseline = 0;
    Size textSize = ft2->getTextSize(to_string(score), fontScale, LINE_AA, &baseline);
    ft2->putText(background, "SCORE: " + to_string(score), Point(10, textSize.height + 10), fontScale, color, cv::FILLED, LINE_AA, true);
}

void displayMessage(Mat& frame, Ptr<freetype::FreeType2>& ft2, Scalar color, const string& message) {
    Mat overlay = Mat::zeros(frame.size(), frame.type());
    overlay.setTo(Scalar(0, 0, 0)); // Fundo preto
    addWeighted(overlay, 0.5, frame, 0.5, 0, frame); // Aplicar transparência

    int fontScale = 80;
    int baseline = 0;
    Size textSize = ft2->getTextSize(message, fontScale, LINE_AA, &baseline);
    Point textOrg((frame.cols - textSize.width) / 2, (frame.rows + textSize.height) / 2);
    
    ft2->putText(frame, message, textOrg, fontScale, color, cv::FILLED, LINE_AA, true);
}

int main() {
    string wName = "CIs Space";

    Mat background = imread("cenarioMenu.png");
    if (background.empty()) {
        cout << "Erro ao carregar o fundo do menu!" << endl;
        return -1;
    }

    Ptr<freetype::FreeType2> ft2 = freetype::createFreeType2();
    ft2->loadFontData("arcadeclassic.ttf", 0);

    Scalar colorTitulo = Scalar(255, 209, 1); 
    Scalar colorMenu = Scalar(255, 255, 255); 
    int centerX = background.cols / 2; 
    int centerY = background.rows / 2; 

    int fontScale = 80;
    int baseline = 0;
    Size textSize = ft2->getTextSize("CIs SPACE", fontScale, LINE_AA, &baseline);
    ft2->putText(background, "CIs SPACE", Point(centerX - textSize.width / 2, centerY - 250), fontScale, colorTitulo, cv::FILLED, LINE_AA, true);

    fontScale = 45;
    textSize = ft2->getTextSize("START", fontScale, LINE_AA, &baseline);
    ft2->putText(background, "START", Point(centerX - textSize.width / 2, centerY - 70), fontScale, colorMenu, cv::FILLED, LINE_AA, true);

    textSize = ft2->getTextSize("CREDITS", fontScale, LINE_AA, &baseline);
    ft2->putText(background, "CREDITS", Point(centerX - textSize.width / 2, centerY), fontScale, colorMenu, cv::FILLED, LINE_AA, true);

    textSize = ft2->getTextSize("EXIT", fontScale, LINE_AA, &baseline);
    ft2->putText(background, "EXIT", Point(centerX - textSize.width / 2, centerY + 70), fontScale, colorMenu, cv::FILLED, LINE_AA, true);

    imshow(wName, background);

    int key = waitKey(0);
    if (key == '1') {
        destroyWindow(wName);

        CascadeClassifier face_cascade;
        if (!face_cascade.load("haarcascade_frontalface_default.xml")) {
            cout << "Erro ao carregar o classificador de rosto!" << endl;
            return -1;
        }

        VideoCapture cap("video.mp4");
        if (!cap.isOpened()) {
            cout << "Erro ao abrir a câmera!" << endl;
            return -1;
        }

        Mat gameBackground = imread("cenarioMenu.png");
        if (gameBackground.empty()) {
            cout << "Erro ao carregar o fundo do jogo!" << endl;
            return -1;
        }

        Mat nave = imread("nave.png", IMREAD_UNCHANGED);
        if (nave.empty()) {
            cout << "Erro ao carregar a imagem da nave!" << endl;
            return -1;
        }
        resize(nave, nave, Size(80, 80));

        Mat shot = imread("Shot.png", IMREAD_UNCHANGED);
        if (shot.empty()) {
            cout << "Erro ao carregar a imagem do tiro!" << endl;
            return -1;
        }
        resize(shot, shot, Size(20, 10));

        Mat target = imread("target.png", IMREAD_UNCHANGED);
        if (target.empty()) {
            cout << "Erro ao carregar a imagem do alvo!" << endl;
            return -1;
        }
        resize(target, target, Size(100, 100));

        Mat explosion = imread("explosion.png", IMREAD_UNCHANGED);
        if (explosion.empty()) {
            cout << "Erro ao carregar a imagem da explosão!" << endl;
            return -1;
        }
        resize(explosion, explosion, Size(80, 80));

        vector<Rect> faces;
        vector<Point> shots; 
        vector<Point> targets; 
        int lastShotTime = 0;
        int score = 0; 
        bool gameOver = false; 
        Point explosionPos; // Posição da explosão
        int hits = 0; // Contador de acertos
        int phase = 1; // Fase atual

        int h = 0;

        
        while (true) {
            if (gameOver) {
                Mat display = gameBackground.clone();
                drawImage(display, explosion, explosionPos.x, explosionPos.y); // Explosão na posição da nave
                imshow(wName, display);
                waitKey(3000); // Espera 3 segundos pela animação

                // Agora desenha a tela "GAME OVER"
                displayMessage(display, ft2, colorMenu, "GAME OVER");
                imshow(wName, display);
                waitKey(3000); // Espera 3 segundos para a tela de GAME OVER
                break; // Volta ao menu
            }

            Mat display = gameBackground.clone();

            if (hits >= 5 || h==0) {
                
                hits = 0; // Reseta o contador de acertos
                displayMessage(display, ft2, colorMenu, "FASE " + to_string(phase)); // Mostra a fase atual
                imshow(wName, display);
                waitKey(3000); // Espera 3 segundos para mostrar a fase
                h++;
                phase++; // Incrementa a fase
                continue;
            }


            Mat frame;
            cap >> frame;
            if (frame.empty()) {
                cout << "Erro ao capturar frame!" << endl;
                break;
            }

            Mat gray;
            cvtColor(frame, gray, COLOR_BGR2GRAY);
            equalizeHist(gray, gray);
            face_cascade.detectMultiScale(gray, faces);

            
            int nave_x = 0, nave_y = 700; 

            if (!faces.empty()) {
                nave_x = faces[0].x + faces[0].width / 2 - nave.cols / 2;
                nave_x = min(max(nave_x, 0), display.cols - nave.cols);
                drawNave(display, nave, nave_x, nave_y);
            }

            for (size_t i = 0; i < shots.size(); i++) {
                shots[i].y -= 15; 
                if (shots[i].y < 0) {
                    shots.erase(shots.begin() + i);
                    i--;
                }
            }

            for (const auto& shotPos : shots) {
                drawShot(display, shot, shotPos.x, shotPos.y);
            }

            // Adiciona novos alvos se houver menos de 10 (ou qualquer número desejado)
            if (targets.size() < 10) {
                int x = rand() % (gameBackground.cols - 100); // Ajusta para o tamanho do alvo
                targets.emplace_back(x, -rand() % 500);
            }

            for (size_t i = 0; i < targets.size(); i++) {
                targets[i].y += 8; 
                if (targets[i].y >= nave_y && targets[i].x + target.cols > nave_x && targets[i].x < nave_x + nave.cols) {
                    gameOver = true; 
                    explosionPos = Point(nave_x, nave_y); // Armazenar a posição da explosão
                }
            }

            for (size_t i = 0; i < shots.size(); i++) {
                for (size_t j = 0; j < targets.size(); j++) {
                    if (abs(shots[i].x - targets[j].x) < 40 && abs(shots[i].y - targets[j].y) < 40) {
                        targets.erase(targets.begin() + j);
                        score += 100; 
                        hits++; // Incrementar o contador de acertos
                        shots.erase(shots.begin() + i);
                        break;
                    }
                }
            }

            // Verifica se o jogador acertou 5 alvos
            
            for (const auto& targetPos : targets) {
                drawTarget(display, target, targetPos.x, targetPos.y);
            }

            drawScore(display, ft2, score, colorMenu);

            imshow(wName, display);
            resizeWindow(wName, 1024, 768);

            auto currentTime = chrono::system_clock::now();
            int elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime.time_since_epoch()).count();
            if (elapsedTime - lastShotTime >= 3) { 
                shots.push_back(Point(nave_x + 45, nave_y - 10)); 
                lastShotTime = elapsedTime; 
            }

            int keyPressed = waitKey(30);
            if (keyPressed == '2') {
                Mat creditsDisplay = display.clone();
                creditsDisplay.setTo(Scalar(0, 0, 0));
                ft2->putText(creditsDisplay, "Feito por Kezia e Rayanne", Point(150, 200), 30, colorMenu, cv::FILLED, LINE_AA, true);
                imshow(wName, creditsDisplay);
                waitKey(3000);
                continue;
            }

            if (keyPressed == 'q' || keyPressed == 27) break;
        }
    } else if (key == '3') {
        cout << "Saindo do jogo..." << endl;
        return 0;
    }

    return 0;
}
