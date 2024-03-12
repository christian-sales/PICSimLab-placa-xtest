#include "board_xtest.h"
#include "../lib/oscilloscope.h"
#include "../lib/picsimlab.h"
#include "../lib/spareparts.h"

enum{
    I_POT1,
    I_ICSP,
    I_PWR,
    I_RST,
    I_BD0,
    I_SD1
};

enum{
    O_POT1,  // potentiometer
    O_RST,   // Reset button
    O_SD1,   // switch position (On/Off)
    O_LD0,   // LED on RD0 push button
    O_LD1,   // LED on RD1 switch
    O_LPWR,  // Power LED
    O_RB0,   // LED on RB0 output
    O_RB1,   // LED on RB1 output
    O_BD0,   // RD1 switch
    O_CPU    // CPU name
};

unsigned short cboard_xtest::GetInputId(char* name) {
    if (strcmp(name, "PG_ICSP") == 0)
        return I_ICSP;
    if (strcmp(name, "SW_PWR") == 0)
        return I_PWR;
    if (strcmp(name, "PB_RST") == 0)
        return I_RST;
    if (strcmp(name, "PB_D0") == 0)
        return I_BD0;
    if (strcmp(name, "SW_D1") == 0)
        return I_SD1;
    if (strcmp(name, "PO_1") == 0)
        return I_POT1;

    printf("Error input '%s' don't have a valid id! \n", name);
    return -1;
}

unsigned short cboard_xtest::GetOutputId(char* name) {
    if (strcmp(name, "SW_D1") == 0)
        return O_SD1;
    if (strcmp(name, "LD_LD0") == 0)
        return O_LD0;
    if (strcmp(name, "LD_LD1") == 0)
        return O_LD1;
    if (strcmp(name, "LD_LPWR") == 0)
        return O_LPWR;
    if (strcmp(name, "LD_RB1") == 0)
        return O_RB1;
    if (strcmp(name, "LD_RB0") == 0)
        return O_RB0;
    if (strcmp(name, "PB_D0") == 0)
        return O_BD0;
    if (strcmp(name, "PO_1") == 0)
        return O_POT1;
    if (strcmp(name, "PB_RST") == 0)
        return O_RST;
    if (strcmp(name, "IC_CPU") == 0)
        return O_CPU;
	
    printf("Error input '%s' don't have a valid id! \n", name);
    return -1;
}

cboard_xtest::cboard_xtest(void) : font(10,lxFONTFAMILY_TELETYPE, lxFONTSTYLE_NORMAL, lxFONTWEIGHT_BOLD) {
    Proc = "PIC18F4550";
    ReadMaps();

    pot1 = 100;
    active = 0;

    if(PICSimLab.GetWindow()){
        //controle de propriedades e criação
        //Medidor 1
        gauge1 = new CGauge();
        gauge1->SetFOwner(PICSimLab.GetWindow());
        gauge1->SetName("gauge1_px");
        gauge1->SetX(13);
        gauge1->SetY(382-160);
        gauge1->SetWidth(140);
        gauge1->SetHeight(20);
        gauge1->SetEnable(1);
        gauge1->SetVisible(1);
        gauge1->SetRange(100);
        gauge1->SetValue(0);
        gauge1->SetType(4);
        PICSimLab.GetWindow()->CreateChild(gauge1);

        //Medidor 2
        gauge2 = new CGauge();
        gauge2->SetFOwner(PICSimLab.GetWindow());
        gauge2->SetName("gauge2_px");
        gauge2->SetX(12);
        gauge2->SetY(330-160);
        gauge2->SetWidth(140);
        gauge2->SetHeight(20);
        gauge2->SetEnable(1);
        gauge2->SetVisible(1);
        gauge2->SetRange(100);
        gauge2->SetValue(0);
        gauge2->SetType(4);
        PICSimLab.GetWindow()->CreateChild(gauge2);

        //Rótulo 2
        label2 = new CLabel();
        label2->SetFOwner(PICSimLab.GetWindow());
        label2->SetName("label2_px");
        label2->SetX(12);
        label2->SetY(306-160);
        label2->SetWidth(60);
        label2->SetHeight(20);
        label2->SetEnable(1);
        label2->SetVisible(1);
        label2->SetAlign(2);
        //label2->SetType(4);
        PICSimLab.GetWindow()->CreateChild(label2);

        //Rótulo 3
        label3 = new CLabel();
        label3->SetFOwner(PICSimLab.GetWindow());
        label3->SetName("label3_px");
        label3->SetX(12);
        label3->SetY(306-160);
        label3->SetWidth(60);
        label3->SetHeight(20);
        label3->SetEnable(1);
        label3->SetVisible(1);
        label3->SetAlign(2);
        //label3->SetType(4);
        PICSimLab.GetWindow()->CreateChild(label3);
    }

    SWBounce_init(&bounce, 2);
}

//Destrutor chamado uma vez na criação da placa

cboard_xtest::~cboard_xtest(void){
    //controla a "destruição"
    if(PICSimLab.GetWindow()){
        PICSimLab.GetWindow()->DestroyChild(gauge1);
        PICSimLab.GetWindow()->DestroyChild(gauge2);
        PICSimLab.GetWindow()->DestroyChild(label2);
        PICSimLab.GetWindow()->DestroyChild(label3);
    }
    SWBounce_end(&bounce);
}

//Resetar o status da placa

void cboard_xtest::Reset(void) {
    pic_reset(&pic, 1);

    p_BT1 = 1;  // set push button  in default state (high)

    // write button state to pic pin 19 (RD0)
    pic_set_pin(&pic, 19, p_BT1);
    // write switch state to pic pin 20 (RD1)
    pic_set_pin(&pic, 20, p_BT2);

    if (PICSimLab.GetStatusBar()) {
        // verify serial port state and refresh status bar
#ifndef _WIN_
        if (pic.serial[0].serialfd > 0)
#else
        if (pic.serial[0].serialfd != INVALID_HANDLE_VALUE)
#endif
            PICSimLab.GetStatusBar()->SetField(
                2, lxT("Serial: ") + lxString::FromAscii(SERIALDEVICE) + lxT(":") + itoa(pic.serial[0].serialbaud) +
                       lxT("(") +
                       lxString().Format("%4.1f",
                                         fabs((100.0 * pic.serial[0].serialexbaud - 100.0 * pic.serial[0].serialbaud) /
                                              pic.serial[0].serialexbaud)) +
                       lxT("%)"));
        else
            PICSimLab.GetStatusBar()->SetField(2,
                                               lxT("Serial: ") + lxString::FromAscii(SERIALDEVICE) + lxT(" (ERROR)"));
    }

    if (use_spare)
        SpareParts.Reset();

    RegisterRemoteControl();
}

//Registra variáveis para serem controladas pelo controle remoto 

void cboard_xtest::RegisterRemoteControl(void){
    //registrar inputs
    input_ids[I_BD0]->status = &p_BT1;
    input_ids[I_SD1]->status = &p_BT2;
    input_ids[I_POT1]->status = &pot1;

    // registra os outputs para serem mudados na mudança de input
    input_ids[I_BD0]->update = &output_ids[O_BD0]->update;
    input_ids[I_SD1]->update = &output_ids[O_SD1]->update;
    input_ids[I_BD0]->update = &output_ids[O_POT1]->update;

    //registrar outputs
    output_ids[O_RB0]->status = &pic.pins[32].oavalue;
    output_ids[O_RB1]->status = &pic.pins[33].oavalue;
    output_ids[O_LD0]->status = &pic.pins[18].oavalue;
    output_ids[O_LD1]->status = &pic.pins[19].oavalue;
}

//Chamado a cada 1s para atualizar o status

void cboard_xtest::RefreshStatus(void) {
    // verify serial port state and refresh status bar
#ifndef _WIN_
    if (pic.serial[0].serialfd > 0)
#else
    if (pic.serial[0].serialfd != INVALID_HANDLE_VALUE)
#endif
        PICSimLab.GetStatusBar()->SetField(2, lxT("Serial: ") + lxString::FromAscii(SERIALDEVICE) + lxT(":") +
                                                  itoa(pic.serial[0].serialbaud) + lxT("(") +
                                                  lxString().Format("%4.1f", fabs((100.0 * pic.serial[0].serialexbaud -
                                                                                   100.0 * pic.serial[0].serialbaud) /
                                                                                  pic.serial[0].serialexbaud)) +
                                                  lxT("%)"));
    else
        PICSimLab.GetStatusBar()->SetField(2, lxT("Serial: ") + lxString::FromAscii(SERIALDEVICE) + lxT(" (ERROR)"));
}

//chamado para salvar as preferências da placa em um arquivo

void cboard_xtest::WritePreferences(void) {
    // write selected microcontroller of board_x to preferences
    PICSimLab.SavePrefs(lxT("X_proc"), Proc);
    // write switch state of board_x to preferences
    PICSimLab.SavePrefs(lxT("X_bt2"), lxString().Format("%i", p_BT2));
    // write microcontroller clock to preferences
    PICSimLab.SavePrefs(lxT("X_clock"), lxString().Format("%2.1f", PICSimLab.GetClock()));
    // write potentiometer position to preferences
    PICSimLab.SavePrefs(lxT("X_pot1"), lxString().Format("%i", pot1));
}

//chamado para carregar as preferências

void cboard_xtest::ReadPreferences(char* name, char* value) {
    // read switch state of board_x of preferences
    if (!strcmp(name, "X_bt2")) {
        if (value[0] == '0')
            p_BT2 = 0;
        else
            p_BT2 = 1;
    }
    // read microcontroller of preferences
    if (!strcmp(name, "X_proc")) {
        Proc = value;
    }
    // read microcontroller clock
    if (!strcmp(name, "X_clock")) {
        PICSimLab.SetClock(atof(value));
    }

    // read potentiometer position
    if (!strcmp(name, "X_pot1")) {
        pot1 = atoi(value);
    }
    if(!strcmp(name, "X_bt2")){
        if(value[0] == '0')
            p_BT2 = 0;
        else
            p_BT2 = 1;
    }
}

//evento na placa

void cboard_xtest::EvKeyPress(uint key, uint mask){
    //se o número 1 for apertado no teclado, então ative o button (state = 0)
    if(key == '1'){
        p_BT1=0;
        output_ids[O_BD0]->update = 1;
    }

    //se o número 2 for apertado então mudar o estado do switch
    if(key=='2'){
        p_BT2 ^=1;
        output_ids[O_SD1]->update = 1;
    }
}

//evento da placa

void cboard_xtest::EvKeyRelease(uint key, uint mask){
    // se for apertado no teclado etnão desativar o botão (state =1 )
    if(key== '1'){
        p_BT1 = 1;
        output_ids[O_BD0]->update = 1;
    }
}

//evento da placa

void cboard_xtest::EvMouseButtonPress(uint button, uint x, uint y, uint state) {
    int i;

    //procurar pela área de input de cada "dono de evento"
    for(i=0; i<inputc;i++){
        if(((input[i].x1 <= x) && (input[i].x2 >= x)) && ((input[i].y1 <= y) && (input[i].y2 >= y))){
            switch (input[i].id) {
                    // Se o evento estiver acima da área do I_ICSP então carregar arquivo hex
                case I_ICSP:
                    PICSimLab.OpenLoadHexFileDialog();
                    ;
                    break;
                    // Se o evento estiver em cima do botão on/off então ligar ou desligar
                case I_PWR:
                    if (PICSimLab.GetMcuPwr())  // if on turn off
                    {
                        PICSimLab.SetMcuPwr(0);
                        Reset();
                        p_BT1 = 1;
                    } else  // se ligado, desligar e ←→
                    {
                        PICSimLab.SetMcuPwr(1);
                        Reset();
                    }
                    output_ids[O_LPWR]->update = 1;
                    break;
                    // Se o evento estiver em cima do botão de reset, então 
                    //desligar e resetar
                case I_RST:
                    if (PICSimLab.GetMcuPwr() && pic_reset(&pic, -1))  // if powered
                    {
                        PICSimLab.SetMcuPwr(0);
                        PICSimLab.SetMcuRst(1);
                    }
                    p_RST = 0;
                    output_ids[O_RST]->update = 1;
                    break;
                    // Se o evento estiver em cima do botão 0, então mudar seu estado
                case I_BD0:
                    p_BT1 = 0;
                    output_ids[O_BD0]->update = 1;
                    break;
                    // Se o evento estiver em cima do potenciômetro, então mudar seu estado
                case I_SD1:
                    p_BT2 ^= 1;
                    output_ids[O_SD1]->update = 1;
                    break;
                case I_POT1: {
                    active = 1;
                    pot1 = (x - input[i].x1) * 2.77;    
                    if (pot1 > 199)
                        pot1 = 199;
                    output_ids[O_POT1]->update = 1;
                } break;
            }
        }
    }
}

void cboard_xtest::EvMouseMove(uint button, uint x, uint y, uint state) {
    int i;
    //Nesse caso, se estiver com o evento do Pot 1 ativado, e o mouse se mover, deve-se alterar o valor do potenciômetro, mas com o limite de 199
    for (i = 0; i < inputc; i++) {
        switch (input[i].id) {
            case I_POT1:
                if (((input[i].x1 <= x) && (input[i].x2 >= x)) && ((input[i].y1 <= y) && (input[i].y2 >= y))) {
                    if (active) {
                        pot1 = (x - input[i].x1) * 2.77;
                        if (pot1 > 199)
                            pot1 = 199;
                        output_ids[O_POT1]->update = 1;
                    }
                } else {
                    active = 0;
                }
                break;
        }
    }
}

void cboard_xtest::EvMouseButtonRelease(uint button, uint x, uint y, uint state) {
    int i;

    // search for the input area which owner the event
    for (i = 0; i < inputc; i++) {
        if (((input[i].x1 <= x) && (input[i].x2 >= x)) && ((input[i].y1 <= y) && (input[i].y2 >= y))) {
            switch (input[i].id) {
                    // if event is over I_RST area then turn on
                case I_RST:
                    if (PICSimLab.GetMcuRst())  // if powered
                    {
                        PICSimLab.SetMcuPwr(1);
                        PICSimLab.SetMcuRst(0);

                        if (pic_reset(&pic, -1)) {
                            Reset();
                        }
                    }
                    p_RST = 1;
                    output_ids[O_RST]->update = 1;
                    break;
                    // Se o evento estiver acima do I_d0, então desativar o botão (state=1)
                case I_BD0:
                    p_BT1 = 1;
                    output_ids[O_BD0]->update = 1;
                    break;
                case I_POT1: {
                    active = 0;
                    output_ids[O_POT1]->update = 1;
                } break;
            }
        }
    }
}


//Chamada a cada 100ms para desenhar a placa
//Parte Essencial para a velocidade do simulador

void cboard_xtest::Draw(CDraw* draw) {
    int update = 0;  // Verificar se é necessário update
    int i;

    // board_x draw
    for (i = 0; i < outputc; i++)  // Procurar por todos outputs
    {
        if (output[i].update)      // Apenas se precisar de atualização
        {
            output[i].update = 0;

            if (!update) {
                draw->Canvas.Init(Scale, Scale);
            }
            update++;                       // atualizar o buffer

            if (!output[i].r)               // se a forma do output é um retângulo
            {
                if (output[i].id == O_SD1)  // se o output é um switch
                {
                    // desenhar um backgroung branco
                    draw->Canvas.SetBgColor(255, 255, 255);
                    draw->Canvas.Rectangle(1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1,
                                           output[i].y2 - output[i].y1);

                    if (!p_BT2)  // desenhar switch desligado
                    {
                        // desenhar um retângulo cinza
                        draw->Canvas.SetBgColor(70, 70, 70);
                        draw->Canvas.Rectangle(
                            1, output[i].x1, output[i].y1 + ((int)((output[i].y2 - output[i].y1) * 0.35)),
                            output[i].x2 - output[i].x1, (int)((output[i].y2 - output[i].y1) * 0.65));
                    } else  // Desenhar o switch ligado
                    {
                        // desenhar um retângulo cinza
                        draw->Canvas.SetBgColor(70, 70, 70);
                        draw->Canvas.Rectangle(1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1,
                                               (int)((output[i].y2 - output[i].y1) * 0.65));
                    }
                } else if (output[i].id == O_BD0) {
                    draw->Canvas.SetColor(102, 102, 102);
                    draw->Canvas.Circle(1, output[i].cx, output[i].cy, 10);
                    if (p_BT1) {
                        draw->Canvas.SetColor(15, 15, 15);
                    } else {
                        draw->Canvas.SetColor(55, 55, 55);
                    }
                    draw->Canvas.Circle(1, output[i].cx, output[i].cy, 8);
                } else if (output[i].id == O_RST) {
                    draw->Canvas.SetColor(102, 102, 102);
                    draw->Canvas.Circle(1, output[i].cx, output[i].cy, 10);

                    if (p_RST) {
                        draw->Canvas.SetColor(15, 15, 15);
                    } else {
                        draw->Canvas.SetColor(55, 55, 55);
                    }
                    draw->Canvas.Circle(1, output[i].cx, output[i].cy, 8);
                } else if (output[i].id == O_POT1) {
                    draw->Canvas.SetColor(0, 50, 215);
                    draw->Canvas.Rectangle(1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1,
                                           output[i].y2 - output[i].y1);
                    draw->Canvas.SetColor(250, 250, 250);
                    draw->Canvas.Rectangle(1, output[i].x1 + pot1 / 2.77, output[i].y1 + 2, 10, 15);
                } else if (output[i].id == O_CPU) {
                    draw->Canvas.SetFont(font);
                    int x, y, w, h;
                    draw->Canvas.SetColor(26, 26, 26);
                    draw->Canvas.Rectangle(1, output[i].x1, output[i].y1, output[i].x2 - output[i].x1,
                                           output[i].y2 - output[i].y1);

                    draw->Canvas.SetColor(230, 230, 230);
                    w = output[i].x2 - output[i].x1;
                    h = output[i].y2 - output[i].y2;
                    x = output[i].x1 + (w / 2) + 7;
                    y = output[i].y1 + (h / 2) + (Proc.length());
                    draw->Canvas.RotatedText(Proc, x, y, 270);
                }
            } else                                 // Se a forma do output é um circulo
            {
                draw->Canvas.SetFgColor(0, 0, 0);  // preto

                switch (output[i].id)              // procurar pela cor do output
                {
                    case O_LD0:                    // Branco usando pino 19 valor médio (RD0)
                        draw->Canvas.SetBgColor(pic.pins[18].oavalue, pic.pins[18].oavalue, pic.pins[18].oavalue);
                        break;
                    case O_LD1:  // Amarelo usando pino 20 valor médio (RD1)
                        draw->Canvas.SetBgColor(pic.pins[19].oavalue, pic.pins[19].oavalue, 0);
                        break;
                    case O_LPWR:  // Azul usando o valor mcupwr
                        draw->Canvas.SetBgColor(0, 0, 200 * PICSimLab.GetMcuPwr() + 55);
                        break;
                    case O_RB0:  // Verde usando pino 33 valor médio (RB0)
                        draw->Canvas.SetBgColor(0, pic.pins[32].oavalue, 0);
                        break;
                    case O_RB1:  // Vermelho usando pino 34 valor médio (RB1)
                        draw->Canvas.SetBgColor(pic.pins[33].oavalue, 0, 0);
                        break;
                }

                // desenhar o LED
                color1 = draw->Canvas.GetBgColor();
                int r = color1.Red() - 120;
                int g = color1.Green() - 120;
                int b = color1.Blue() - 120;
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;
                color2.Set(r, g, b);
                draw->Canvas.SetBgColor(color2);
                draw->Canvas.Circle(1, output[i].x1, output[i].y1, output[i].r + 1);
                draw->Canvas.SetBgColor(color1);
                draw->Canvas.Circle(1, output[i].x1, output[i].y1, output[i].r - 2);
            }
        }
    }
        // terminar desenho

    if (update) {
        draw->Canvas.End();
        draw->Update();
    }

    // valor médio de RB0 para gauge1
    gauge1->SetValue((pic.pins[33].oavalue - 55) / 2);
    // Valor médio de RB1 para gauge2
    gauge2->SetValue((pic.pins[32].oavalue - 55) / 2);
}


void cboard_xtest::Run_CPU(void) {
    int i;
    int j;
    unsigned char pi;
    const picpin* pins;
    unsigned int alm[40];
    int bret;

    const int JUMPSTEPS = PICSimLab.GetJUMPSTEPS();  //número de passos ignorados
    const long int NSTEP = PICSimLab.GetNSTEP();     //número de passos em 100ms
    const float RNSTEP = 200.0 * pic.PINCOUNT / NSTEP;

    // redefinir valor médio dos pinos
    memset(alm, 0, 40 * sizeof(unsigned int));

    // Leia pic.pins para uma variável local para acelerar
    pins = pic.pins;

    // Pré-processamento de partes sobressalentes da janela
    if (use_spare)
        SpareParts.PreProcess();

    SWBounce_prepare(&bounce, PICSimLab.GetBoard()->MGetInstClockFreq());

    unsigned char p_BT1_ = p_BT1;
    unsigned char p_BT2_ = p_BT2;

    if ((pins[19 - 1].dir == PD_IN) && (pins[19 - 1].value != p_BT1_)) {
        SWBounce_bounce(&bounce, 0);
    }
    if ((pins[20 - 1].dir == PD_IN) && (pins[20 - 1].value != p_BT2_)) {
        SWBounce_bounce(&bounce, 1);
    }

    j = JUMPSTEPS;                   // contador de passos
    pi = 0;
    if (PICSimLab.GetMcuPwr())       // se energizado
        for (i = 0; i < NSTEP; i++)  // repita para o número de passos em 100ms
        {
            if (j >= JUMPSTEPS)      // se o número de etapas for maior que o números de etapas ignoradas...
            {
                pic_set_pin(&pic, pic.mclr, p_RST);
                if (!bounce.do_bounce) {
                    pic_set_pin(&pic, 19, p_BT1_);  // Defina o pino 19 (RD0) com o estado do botão
                    pic_set_pin(&pic, 20, p_BT2_);  // Defina o pino 20 (RD1) com o estado do switch
                }
            }

            if (bounce.do_bounce) {
                bret = SWBounce_process(&bounce);
                if (bret) {
                    if (bounce.bounce[0]) {
                        if (bret == 1) {
                            pic_set_pin(&pic, 19, !pins[19 - 1].value);
                        } else {
                            pic_set_pin(&pic, 19, p_BT1_);
                        }
                    }
                    if (bounce.bounce[1]) {
                        if (bret == 1) {
                            pic_set_pin(&pic, 20, !pins[20 - 1].value);
                        } else {
                            pic_set_pin(&pic, 20, p_BT2_);
                        }
                    }
                }
            }

            // verifique se um ponto de interrupção foi alcançado se não executar uma instrução
            if (!mplabxd_testbp())
                pic_step(&pic);
            ioupdated = pic.ioupdated;
            InstCounterInc();
            // Processo da janela do osciloscópio
            if (use_oscope)
                Oscilloscope.SetSample();
            //Processo das partes sobressalentes da janela
            if (use_spare)
                SpareParts.Process();

            // incrementar o contador do valor médio se o pino estiver alto
            alm[pi] += pins[pi].value;
            pi++;
            if (pi == pic.PINCOUNT)
                pi = 0;

            if (j >= JUMPSTEPS)  //  se o número de etapas for maior que o números de etapas ignoradas...
            {
                // defina o pino 2 analógico (AN0) com o valor do scroll
                pic_set_apin(&pic, 2, (5.0 * pot1 / 199));

                j = -1;  // resetar o contador
            }
            j++;         // incrementar o contador
            pic.ioupdated = 0;
        }

    // calcule o valor médio
    for (pi = 0; pi < pic.PINCOUNT; pi++) {
        pic.pins[pi].oavalue = (int)((alm[pi] * RNSTEP) + 55);
    }

    // Pré pós processamento das partes sobressalentes da janela
    if (use_spare)
        SpareParts.PostProcess();

    // verifique se os leds precisam de atualização
    if (output_ids[O_LD0]->value != pic.pins[18].oavalue) {
        output_ids[O_LD0]->value = pic.pins[18].oavalue;
        output_ids[O_LD0]->update = 1;
    }
    if (output_ids[O_LD1]->value != pic.pins[19].oavalue) {
        output_ids[O_LD1]->value = pic.pins[19].oavalue;
        output_ids[O_LD1]->update = 1;
    }
    if (output_ids[O_RB0]->value != pic.pins[32].oavalue) {
        output_ids[O_RB0]->value = pic.pins[32].oavalue;
        output_ids[O_RB0]->update = 1;
    }
    if (output_ids[O_RB1]->value != pic.pins[33].oavalue) {
        output_ids[O_RB1]->value = pic.pins[33].oavalue;
        output_ids[O_RB1]->update = 1;
    }
}

// Registrar a placa no PICSimlab
board_init(BOARD_xtest_Name, cboard_xtest);
