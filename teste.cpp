#include <opencv2/opencv.hpp> // Inclui a biblioteca OpenCV para manipulação de imagens e vídeos.
#include <opencv2/freetype.hpp> // Inclui suporte para renderização de texto com FreeType.
#include <opencv2/objdetect.hpp> // Inclui suporte para detecção de objetos, como rostos.
#include <iostream> // Inclui a biblioteca de entrada/saída padrão do C++.
#include <thread> // Inclui suporte para manipulação de threads (não utilizado neste código).
#include <vector> // Inclui a biblioteca para usar vetores dinâmicos.
#include <chrono> // Inclui suporte para manipulação de tempo.
#include <cstdlib> // Inclui funções de utilidade, como rand() e system().

using namespace cv; // Usa o namespace da OpenCV.
using namespace std; // Usa o namespace padrão do C++.

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

        VideoCapture cap("video.mp4"); // Abre o vídeo.
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
            cout << "Erro ao carregar a imagem da nave!" << endl; // Mensagem de erro.
            return -1; // Encerra o programa se houver erro.
        }
        resize(nave, nave, Size(80, 80)); // Redimensiona a nave.

        Mat shot = imread("Shot.png",

 IMREAD_UNCHANGED); // Carrega a imagem do tiro.
        if (shot.empty()) { // Verifica se o tiro foi carregado corretamente.
            cout << "Erro ao carregar a imagem do tiro!" << endl; // Mensagem de erro.
            return -1; // Encerra o programa se houver erro.
        }
        resize(shot, shot, Size(20, 10)); // Redimensiona o tiro.

        Mat target = imread("target.png", IMREAD_UNCHANGED); // Carrega a imagem do alvo.
        if (target.empty()) { // Verifica se o alvo foi carregado corretamente.
            cout << "Erro ao carregar a imagem do alvo!" << endl; // Mensagem de erro.
            return -1; // Encerra o programa se houver erro.
        }
        resize(target, target, Size(100, 100)); // Redimensiona o alvo.

        Mat explosion = imread("explosion.png", IMREAD_UNCHANGED); // Carrega a imagem da explosão.
        if (explosion.empty()) { // Verifica se a explosão foi carregada corretamente.
            cout << "Erro ao carregar a imagem da explosão!" << endl; // Mensagem de erro.
            return -1; // Encerra o programa se houver erro.
        }
        resize(explosion, explosion, Size(80, 80)); // Redimensiona a explosão.

        vector<Rect> faces; // Vetor para armazenar rostos detectados.
        vector<Point> shots; // Vetor para armazenar as posições dos tiros.
        vector<Point> targets; // Vetor para armazenar as posições dos alvos.
        int lastShotTime = 0; // Armazena o tempo do último tiro.
        int score = 0; // Inicializa a pontuação do jogador.
        bool gameOver = false; // Indica se o jogo acabou.
        Point explosionPos; // Posição da explosão.
        int hits = 0; // Contador de acertos.
        int phase = 1; // Fase atual do jogo.

        int h = 0; // Contador de fases.

        while (true) { // Loop principal do jogo.
            if (gameOver) { // Se o jogo acabou.
                Mat display = gameBackground.clone(); // Clona o fundo do jogo.
                drawImage(display, explosion, explosionPos.x, explosionPos.y); // Desenha a explosão.
                imshow(wName, display); // Mostra a explosão.
                waitKey(3000); // Espera 3 segundos.

                // Desenha a tela "GAME OVER".
                displayMessage(display, ft2, colorMenu, "GAME OVER"); // Chama a função para exibir a mensagem de Game Over.
                imshow(wName, display); // Mostra a mensagem.
                waitKey(3000); // Espera 3 segundos para mostrar a tela de GAME OVER.
                break; // Sai do loop e volta ao menu.
            }

            Mat display = gameBackground.clone(); // Clona o fundo do jogo.

            if (hits >= 5 || h==0) { // Se o jogador acertou 5 alvos ou é a primeira fase.
                hits = 0; // Reseta o contador de acertos.
                displayMessage(display, ft2, colorMenu, "FASE " + to_string(phase)); // Mostra a fase atual.
                imshow(wName, display); // Exibe a fase.
                waitKey(3000); // Espera 3 segundos.
                h++; // Incrementa o contador de fases.
                phase++; // Avança para a próxima fase.
                continue; // Volta ao início do loop.
            }

            Mat frame; // Matriz para armazenar cada frame do vídeo.
            cap >> frame; // Captura o próximo frame do vídeo.
            if (frame.empty()) { // Verifica se o frame foi capturado corretamente.
                cout << "Erro ao capturar frame!" << endl; // Mensagem de erro.
                break; // Sai do loop se houver erro.
            }

            Mat gray; // Matriz para a imagem em escala de cinza.
            cvtColor(frame, gray, COLOR_BGR2GRAY); // Converte o frame para escala de cinza.
            equalizeHist(gray, gray); // Equaliza o histograma da imagem em escala de cinza para melhorar o contraste.
            face_cascade.detectMultiScale(gray, faces); // Detecta rostos na imagem em escala de cinza.

            int nave_x = 0, nave_y = 700; // Posições iniciais da nave.

            if (!faces.empty()) { // Se rostos foram detectados.
                nave_x = faces[0].x + faces[0].width / 2 - nave.cols / 2; // Posiciona a nave em relação ao rosto detectado.
                nave_x = min(max(nave_x, 0), display.cols - nave.cols); // Garante que a nave não saia dos limites.
                drawNave(display, nave, nave_x, nave_y); // Desenha a nave na tela.
            }

            for (size_t i = 0; i < shots.size(); i++) { // Atualiza a posição dos tiros.
                shots[i].y -= 15; // Move o tiro para cima.
                if (shots[i].y < 0) { // Se o tiro sai da tela.
                    shots.erase(shots.begin() + i); // Remove o tiro do vetor.
                    i--; // Decrementa o índice para evitar pular tiros.
                }
            }

            for (const auto& shotPos : shots) { // Desenha todos os tiros na tela.
                drawShot(display, shot, shotPos.x, shotPos.y);
            }

            // Adiciona novos alvos se houver menos de 10.
            if (targets.size() < 10) {
                int x = rand() % (gameBackground.cols - 100); // Gera posição aleatória para o alvo.
                targets.emplace_back(x, -rand() % 500); // Adiciona o alvo no vetor, começando fora da tela.
            }

            for (size_t i = 0; i < targets.size(); i++) { // Atualiza a posição dos alvos.
                targets[i].y += 8; // Move o alvo para baixo.
                // Verifica se o alvo atingiu a nave.
                if (targets[i].y >= nave_y && targets[i].x + target.cols > nave_x && targets[i].x < nave_x + nave.cols) {
                    gameOver = true; // Se atingiu, o jogo acaba.
                    explosionPos = Point(nave_x, nave_y); // Armazena a posição da explosão.
                }
            }

            for (size_t i = 0; i < shots.size(); i++) { // Verifica colisões entre tiros e alvos.
                for (size_t j = 0; j < targets.size(); j++) {
                    // Se o tiro atinge o alvo.
                    if (abs(shots[i].x - targets[j].x) < 40 && abs(shots[i].y - targets[j].y) < 40) {
                        targets.erase(targets.begin() + j); // Remove o alvo.
                        score += 100; // Incrementa a pontuação.
                        hits++; // Incrementa o contador de acertos.
                        shots.erase(shots.begin() + i); // Remove o tiro.
                        break; // Sai do loop para evitar múltiplas colisões.
                    }
                }
            }

            // Desenha todos os alvos na tela.
            for (const auto& targetPos : targets) {
                drawTarget(display, target, targetPos.x, targetPos.y);
            }

            drawScore(display, ft2, score, colorMenu); // Desenha a pontuação na tela.

            imshow(wName, display); // Mostra a tela do jogo.
            resizeWindow(wName, 1024, 768); // Redimensiona a janela.

            // Checa o tempo para adicionar novos tiros.
            auto currentTime = chrono::system_clock::now(); // Obtém o tempo atual.
            int elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime.time_since_epoch()).count(); // Calcula o tempo desde o início.
            if (elapsedTime - lastShotTime >= 3) { // Se passaram 3 segundos desde o último tiro.
                shots.push_back(Point(nave_x + 45, nave_y - 10)); // Adiciona um novo tiro.
                lastShotTime = elapsedTime; // Atualiza o tempo do último tiro.
            }

            int keyPressed = waitKey(30); // Espera por uma tecla e controla a taxa de frames.
            if (keyPressed == '2') { // Se a tecla '2' for pressionada.
                Mat creditsDisplay = display.clone(); // Clona a tela atual para exibir créditos.
                creditsDisplay.setTo(Scalar(0, 0, 0)); // Preenche a tela de créditos com preto.
                ft2->putText(creditsDisplay, "Feito por Kezia e Rayanne", Point(150, 200), 30, colorMenu, cv::FILLED, LINE_AA, true); // Desenha os créditos.
                imshow(wName, creditsDisplay); // Mostra a tela de créditos.
                waitKey(3000); // Espera 3 segundos.
                continue; // Volta ao início do loop.
            }

            if (keyPressed == 'q' || keyPressed == 27) break; // Se 'q' ou 'ESC' for pressionado, sai do loop.
       

 }
    } else if (key == '3') { // Se a tecla '3' for pressionada.
        cout << "Saindo do jogo..." << endl; // Mensagem de saída.
        return 0; // Encerra o programa.
    }

    return 0; // Retorno final do programa.
}
