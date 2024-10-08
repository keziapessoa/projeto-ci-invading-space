#include <opencv2/opencv.hpp>
#include <opencv2/freetype.hpp>  // Para a fonte arcadeclassic.ttf
#include <opencv2/objdetect.hpp> // Para detecção de rosto
#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>

using namespace cv;
using namespace std;

void drawImage(Mat frame, Mat img, int xPos, int yPos) {
    if (yPos + img.rows >= frame.rows || xPos + img.cols >= frame.cols)
        return;

    Mat mask;
    vector<Mat> layers;

    split(img, layers); // seperate channels
    if (layers.size() == 4) { // img com transparencia.
        Mat rgb[3] = { layers[0],layers[1],layers[2] };
        mask = layers[3]; // png's alpha channel used as mask
        merge(rgb, 3, img);  // put together the RGB channels, now transp insn't transparent 
        img.copyTo(frame.rowRange(yPos, yPos + img.rows).colRange(xPos, xPos + img.cols), mask);
    } else {
        img.copyTo(frame.rowRange(yPos, yPos + img.rows).colRange(xPos, xPos + img.cols));
    }
}

// Função para desenhar a nave com transparência
void drawNave(Mat& background, Mat& nave, int x, int y) {
    for (int i = 0; i < nave.rows; i++) {
        for (int j = 0; j < nave.cols; j++) {
            Vec4b& pixel = nave.at<Vec4b>(i, j);
            if (pixel[3] > 0) {  // Se o canal alfa for maior que 0 (não transparente)
                background.at<Vec3b>(y + i, x + j) = Vec3b(pixel[0], pixel[1], pixel[2]);  // Copia a cor da nave
            }
        }
    }
}


void drawShot(Mat background, Mat shot, int x, int y) {
    for (int i = y; i > 0; i--) {
     drawImage(background, shot, x, i);   
    }
    
}





int main() {
    // Nome da janela
    string wName = "CI Invading Space";
    vector<thread> threads;
    
    // Carregar o background do menu
    Mat background = imread("cenario_Exosfera.png");
    if (background.empty()) {
        cout << "Erro ao carregar o fundo do menu!" << endl;
        return -1;
    }

    // Criar um objeto para FreeType2
    Ptr<freetype::FreeType2> ft2 = freetype::createFreeType2();
    ft2->loadFontData("arcadeclassic.ttf", 0);  // Carregar a fonte arcadeclassic.ttf

    // Cor do texto (branco)
    Scalar color = Scalar(255, 255, 255);

    // Desenhar o título centralizado no menu
    ft2->putText(background, "CI Invading Space", Point(150, 100), 40, color, cv::FILLED, LINE_AA, true);
    
    // Desenhar as opções do menu
    ft2->putText(background, "1. Start", Point(200, 300), 30, color, cv::FILLED, LINE_AA, true);
    ft2->putText(background, "2. Credits", Point(200, 350), 30, color, cv::FILLED, LINE_AA, true);
    ft2->putText(background, "3. Exit", Point(200, 400), 30, color, cv::FILLED, LINE_AA, true);

    // Mostrar o menu com fundo e opções
    imshow(wName, background);
    
    int key = waitKey(0);  // Espera por uma tecla

    if (key == '1') {
        // Fecha o menu para iniciar o jogo
        destroyWindow(wName);  // Fecha a janela do menu

        // Iniciar o jogo

        // Carregar o classificador de rosto
        CascadeClassifier face_cascade;
        if (!face_cascade.load("haarcascade_frontalface_default.xml")) {
            cout << "Erro ao carregar o classificador de rosto!" << endl;
            return -1;
        }

        // Abre a câmera
        VideoCapture cap("video.mp4");
        if (!cap.isOpened()) {
            cout << "Erro ao abrir a câmera!" << endl;
            return -1;
        }

        // Carregar o cenário do jogo (opcional, se o fundo do jogo for diferente do menu)
        Mat gameBackground = imread("cenario_Troposfera.png");
        if (gameBackground.empty()) {
            cout << "Erro ao carregar o fundo do jogo!" << endl;
            return -1;
        }

        // Carregar a nave (sprite da nave) com canal alfa
        Mat nave = imread("nave.png", IMREAD_UNCHANGED);
        if (nave.empty()) {
            cout << "Erro ao carregar a imagem da nave!" << endl;
            return -1;
        }
        resize(nave, nave, Size(100, 100));  // Redimensionar a nave para 100x100 pixels

        Mat shot = imread("Shot.png", IMREAD_UNCHANGED);
        if (nave.empty()) {
            cout << "Erro ao carregar a imagem do tiro!" << endl;
            return -1;
        }

        Mat frame;
        vector<Rect> faces;

        // Loop principal do jogo
        while (true) {
            cap >> frame;
            if (frame.empty()) {
                cout << "Erro ao capturar frame!" << endl;
                break;
            }

            // Converter para escala de cinza para detectar rosto
            Mat gray;
            cvtColor(frame, gray, COLOR_BGR2GRAY);
            equalizeHist(gray, gray);

            // Detectar o rosto
            face_cascade.detectMultiScale(gray, faces);

            // Colocar o fundo do jogo (cenário) no display
            Mat display = gameBackground.clone();  // Clonar o fundo do jogo

            // Se detectar um rosto, mova a nave de acordo com a posição do rosto
            if (!faces.empty()) {
                int face_x = faces[0].x + faces[0].width / 2 - nave.cols / 2;  // Centraliza a nave no rosto
                int face_y = faces[0].y + faces[0].height / 2 - nave.rows / 2;

                // Limitar a nave a não sair da tela
                face_x = min(max(face_x, 0), display.cols - nave.cols);
                face_y = min(max(face_y, 0), display.rows - nave.rows);

                // Desenhar a nave na posição baseada no rosto
                drawNave(display, nave, face_x, 700);
                threads.push_back(thread(drawShot,display, shot, face_x + 45, 695));
            }

            // Mostrar o jogo com o fundo e a nave
            imshow(wName, display);

            // Adicionar a lógica para mostrar créditos se a tecla 2 for pressionada
            int keyPressed = waitKey(30); // Captura a tecla pressionada
            if (keyPressed == '2') {
                Mat creditsDisplay = display.clone(); // Clonar a imagem atual para mostrar os créditos
                creditsDisplay.setTo(Scalar(0, 0, 0)); // Define fundo preto para a tela de créditos
                ft2->putText(creditsDisplay, "Feito por Kezia e Rayanne", Point(150, 200), 30, color, cv::FILLED, LINE_AA, true);
                imshow(wName, creditsDisplay);
                waitKey(3000); // Espera 3 segundos para mostrar os créditos
                continue; // Retorna ao loop principal
            }

            // Sai do loop se 'q' for pressionado
            if (keyPressed == 'q' || keyPressed == 27) break;
        }
    } else if (key == '3') {
        cout << "Saindo do jogo..." << endl;
        return 0;  // Sai do programa ao pressionar 3
    }

    return 0;
}
