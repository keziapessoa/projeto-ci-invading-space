#include <opencv2/opencv.hpp> // Inclui a biblioteca OpenCV para manipulação de imagens e vídeos.
#include <opencv2/freetype.hpp> // Inclui suporte para renderização de texto com FreeType.
#include <opencv2/objdetect.hpp> // Inclui suporte para detecção de objetos, como rostos.
#include <iostream> // Inclui a biblioteca de entrada/saída padrão do C++.
#include <vector> // Inclui a biblioteca para usar vetores dinâmicos.
#include <chrono> // Inclui suporte para manipulação de tempo.
#include <cstdlib> // Inclui funções de utilidade, como rand() e system().

using namespace cv;
using namespace std;

void drawImage(Mat frame, Mat img, int xPos, int yPos) {
    // Verifica se a imagem a ser desenhada está fora dos limites do quadro.
    if (yPos + img.rows >= frame.rows || xPos + img.cols >= frame.cols || yPos < 0 || xPos < 0)
        return; // Se estiver fora, retorna sem desenhar.

    Mat mask; // Máscara para a imagem (transparente, se houver).
    vector<Mat> layers; // Camadas da imagem (se tiver um canal alfa).

    split(img, layers); // Separa a imagem em canais.
    if (layers.size() == 4) { // Se houver 4 camadas (RGBA).
        Mat rgb[3] = { layers[0], layers[1], layers[2] }; // RGB.
        mask = layers[3]; // Canal alfa como máscara.
        merge(rgb, 3, img); // Recria a imagem RGB sem o canal alfa.
        img.copyTo(frame.rowRange(yPos, yPos + img.rows).colRange(xPos, xPos + img.cols), mask); // Desenha usando a máscara.
    } else {
        img.copyTo(frame.rowRange(yPos, yPos + img.rows).colRange(xPos, xPos + img.cols)); // Desenha normalmente.
    }
}

void drawNave(Mat& background, Mat& nave, int x, int y) {
    drawImage(background, nave, x, y); // Chama a função drawImage para desenhar a nave.
}

void drawShot(Mat& background, Mat& shot, int x, int y) {
    drawImage(background, shot, x, y); // Chama a função drawImage para desenhar o tiro.
}

void drawTarget(Mat& background, Mat& target, int x, int y) {
    drawImage(background, target, x, y); // Chama a função drawImage para desenhar o alvo.
}

void drawScore(Mat& background, Ptr<freetype::FreeType2>& ft2, int score, Scalar color) {
    int fontScale = 30; // Tamanho da fonte.
    int baseline = 0; // Baseline da fonte (ajuste vertical).
    Size textSize = ft2->getTextSize(to_string(score), fontScale, LINE_AA, &baseline); // Calcula o tamanho do texto.
    ft2->putText(background, "SCORE: " + to_string(score), Point(10, textSize.height + 10), fontScale, color, cv::FILLED, LINE_AA, true); // Desenha o texto no fundo.
}

void displayMessage(Mat& frame, Ptr<freetype::FreeType2>& ft2, Scalar color, const string& message) {
    Mat overlay = Mat::zeros(frame.size(), frame.type()); // Cria uma sobreposição preta do mesmo tamanho que o quadro.
    overlay.setTo(Scalar(0, 0, 0)); // Preenche a sobreposição de preto.
    addWeighted(overlay, 0.5, frame, 0.5, 0, frame); // Aplica transparência à sobreposição sobre o quadro.

    int fontScale = 80; // Tamanho da fonte.
    int baseline = 0; // Baseline da fonte.
    Size textSize = ft2->getTextSize(message, fontScale, LINE_AA, &baseline); // Calcula o tamanho do texto.
    Point textOrg((frame.cols - textSize.width) / 2, (frame.rows + textSize.height) / 2); // Centraliza o texto.

    ft2->putText(frame, message, textOrg, fontScale, color, cv::FILLED, LINE_AA, true); // Desenha a mensagem no quadro.
}

int main() {
    string wName = "CIs Space"; // Nome da janela.

    Mat background = imread("cenarioMenu.png"); // Carrega o fundo do menu.
    if (background.empty()) { // Verifica se o fundo foi carregado corretamente.
        cout << "Erro ao carregar o fundo do menu!" << endl; // Mensagem de erro.
        return -1; // Encerra o programa se houver erro.
    }

    Ptr<freetype::FreeType2> ft2 = freetype::createFreeType2(); // Cria um objeto FreeType2 para renderização de texto.
    ft2->loadFontData("arcadeclassic.ttf", 0); // Carrega a fonte.

    Scalar colorTitulo = Scalar(255, 209, 1); // Define a cor do título.
    Scalar colorMenu = Scalar(255, 255, 255); // Define a cor do menu.
    int centerX = background.cols / 2; // Calcula a posição central em X.
    int centerY = background.rows / 2; // Calcula a posição central em Y.

    int fontScale = 80; // Tamanho da fonte do título.
    int baseline = 0; // Baseline da fonte.
    Size textSize = ft2->getTextSize("CIs SPACE", fontScale, LINE_AA, &baseline); // Calcula o tamanho do título.
    ft2->putText(background, "CIs SPACE", Point(centerX - textSize.width / 2, centerY - 250), fontScale, colorTitulo, cv::FILLED, LINE_AA, true); // Desenha o título no fundo.

    fontScale = 45; // Tamanho da fonte para opções de menu.
    textSize = ft2->getTextSize("START", fontScale, LINE_AA, &baseline); // Calcula o tamanho da opção "START".
    ft2->putText(background, "START", Point(centerX - textSize.width / 2, centerY - 70), fontScale, colorMenu, cv::FILLED, LINE_AA, true); // Desenha a opção "START".

    textSize = ft2->getTextSize("CREDITS", fontScale, LINE_AA, &baseline); // Calcula o tamanho da opção "CREDITS".
    ft2->putText(background, "CREDITS", Point(centerX - textSize.width / 2, centerY), fontScale, colorMenu, cv::FILLED, LINE_AA, true); // Desenha a opção "CREDITS".

    textSize = ft2->getTextSize("EXIT", fontScale, LINE_AA, &baseline); // Calcula o tamanho da opção "EXIT".
    ft2->putText(background, "EXIT", Point(centerX - textSize.width / 2, centerY + 70), fontScale, colorMenu, cv::FILLED, LINE_AA, true); // Desenha a opção "EXIT".

    imshow(wName, background); // Exibe o fundo do menu na janela.

    int key = waitKey(0); // Espera por uma tecla ser pressionada.
    if (key == '1') { // Se a tecla '1' for pressionada.
        destroyWindow(wName); // Fecha a janela do menu.

        CascadeClassifier face_cascade; // Classificador de rostos.
        if (!face_cascade.load("haarcascade_frontalface_default.xml")) { // Tenta carregar o classificador de rostos.
            cout << "Erro ao carregar o classificador de rosto!" << endl; // Mensagem de erro.
            return -1; // Encerra o programa se houver erro.
        }

        VideoCapture cap(0); // Abre o vídeo.
        //VideoCapture cap("rtsp://192.168.42.117:8080/h264_ulaw.sdp"); // Abre o vídeo.
        if (!cap.isOpened()) { // Verifica se o vídeo foi aberto corretamente.
            cout << "Erro ao abrir a câmera!" << endl; // Mensagem de erro.
            return -1; // Encerra o programa se houver erro.
        }

        Mat gameBackground = imread("cenarioMenu.png"); // Carrega o fundo do jogo.
        if (gameBackground.empty()) { // Verifica se o fundo foi carregado corretamente.
            cout << "Erro ao carregar o fundo do jogo!" << endl; // Mensagem de erro.
            return -1; // Encerra o programa se houver erro.
        }

        Mat nave = imread("nave.png", IMREAD_UNCHANGED); // Carrega a imagem da nave.
        if (nave.empty()) { // Verifica se a nave foi carregada corretamente.
            cout << "Erro ao carregar a nave!" << endl; // Mensagem de erro.
            return -1; // Encerra o programa se houver erro.
        }

        Mat shot = imread("shot.png", IMREAD_UNCHANGED); // Carrega a imagem do tiro.
        if (shot.empty()) { // Verifica se o tiro foi carregado corretamente.
            cout << "Erro ao carregar o tiro!" << endl; // Mensagem de erro.
            return -1; // Encerra o programa se houver erro.
        }

        Mat target = imread("target.png", IMREAD_UNCHANGED); // Carrega a imagem do alvo.
        if (target.empty()) { // Verifica se o alvo foi carregado corretamente.
            cout << "Erro ao carregar o alvo!" << endl; // Mensagem de erro.
            return -1; // Encerra o programa se houver erro.
        }

        // Configurações do jogo.
        int naveX = 0; // Posição inicial da nave em X.
        int naveY = 0; // Posição inicial da nave em Y.
        int score = 0; // Pontuação do jogador.
        int shotX = -1, shotY = -1; // Posição do tiro.
        bool isShotFired = false; // Verifica se o tiro foi disparado.

        while (true) { // Loop principal do jogo.
            Mat frame; // Matriz para o quadro da câmera.
            cap >> frame; // Captura o quadro da câmera.
            if (frame.empty()) break; // Se o quadro estiver vazio, sai do loop.

            flip(frame, frame, 1); // Inverte o quadro horizontalmente.

            Mat gray; // Matriz para o quadro em escala de cinza.
            cvtColor(frame, gray, COLOR_BGR2GRAY); // Converte o quadro para escala de cinza.

            vector<Rect> faces; // Vetor para armazenar as faces detectadas.
            face_cascade.detectMultiScale(gray, faces, 1.1, 4); // Detecta rostos.

            // Desenho da nave e lógica de disparo.
            if (!faces.empty()) { // Se rostos foram detectados.
                naveX = faces[0].x + faces[0].width / 2 - nave.cols / 2; // Centraliza a nave na face detectada.
                naveY = faces[0].y + faces[0].height; // Coloca a nave na parte inferior da face detectada.
                drawNave(frame, nave, naveX, naveY); // Desenha a nave.

                if (!isShotFired) { // Se o tiro não foi disparado.
                    shotX = naveX + nave.cols / 2 - shot.cols / 2; // Centraliza o tiro na nave.
                    shotY = naveY; // Posição Y do tiro.
                    isShotFired = true; // Marca que o tiro foi disparado.
                }
            } else {
                naveX = -100; // Posiciona a nave fora da tela se nenhum rosto for detectado.
                naveY = -100; // Coloca a nave fora da tela.
            }

            // Lógica do tiro.
            if (isShotFired) {
                drawShot(frame, shot, shotX, shotY); // Desenha o tiro.
                shotY -= 10; // Move o tiro para cima.
                if (shotY < 0) { // Se o tiro sair da tela.
                    isShotFired = false; // Reseta o tiro.
                }
            }

            drawScore(frame, ft2, score, Scalar(255, 255, 255)); // Desenha a pontuação.

            // Lógica do alvo.
            if (shotY >= 0) { // Se o tiro estiver na tela.
                drawTarget(frame, target, centerX - target.cols / 2, 50); // Desenha o alvo na parte superior da tela.
                if (shotX >= centerX - target.cols / 2 && shotX <= centerX + target.cols / 2 && shotY <= 50) {
                    score += 10; // Aumenta a pontuação ao acertar o alvo.
                    isShotFired = false; // Reseta o tiro após acertar.
                    shotY = -1; // Retira o tiro da tela.
                }
            }

            imshow("CIs Space", frame); // Exibe o quadro processado.
            if (waitKey(30) >= 0) break; // Aguarda uma tecla por 30 ms e sai do loop se uma tecla for pressionada.
        }

        cap.release(); // Libera a captura da câmera.
    }

    return 0; // Finaliza o programa.
}
