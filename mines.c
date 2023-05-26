
#include "mines.h"




void generate(mines *m){
    /*randomly generates playfield*/
    //generate bombs
    for(int i = 0; i < 100; i++){
        m->data[i] = 0;
        m->revealed[i] = 0;
    }
    int bombCount = 0;
    srand(time(NULL));
    while(bombCount < 10){
        int r = rand()%100;
        if(m->data[r] != -1){
            m->data[r] = -1;
            bombCount++;
        }
    } 
    //generate numbers
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++){
            if (m->data[i*10+j] != -1){
                int count = 0;
                if( i != 0){
                    if (m->data[(i-1)*10+j] == -1){
                        count += 1;
                }}
                if( i != 9){
                    if (m->data[(i+1)*10+j] == -1){
                        count += 1;
                }}
                if (j != 0){
                    if (m->data[(i)*10+j-1] == -1){
                        count += 1;
                }
                }
                if (j != 9){
                    if (m->data[(i)*10+j+1] == -1){
                        count += 1;
                    }
                }
                if (i != 0 && j != 0){
                    if (m->data[(i-1)*10+j-1] == -1){
                        count += 1;
                    }
                }
                if (i != 0 && j != 9){
                    if (m->data[(i-1)*10+j+1] == -1){
                        count += 1;
                    }
                }
                if (i != 9 && j != 0){
                    if (m->data[(i+1)*10+j-1] == -1){
                        count += 1;
                    }
                }
                if (i != 9 && j != 9){
                    if (m->data[(i+1)*10+j+1] == -1){
                        count += 1;
                    }
                }
                m->data[i*10+j] = count;
            }
        }
    }
}

void reveal_part(mines *m, queue_t *que, int idx){
    if (m->data[idx] == 0 && m->revealed[idx] == 0){
        if (!contains(que, idx)){
            push_to_queue(que, idx);
        }
    }
    if(m->revealed[idx] == 0){ //
        m->revealed[idx] = 1;
        m->score++;
    }
    
}

bool reveal(mines *m){
    bool result = false;
    if(m->data[m->selectedY*10+m->selectedX] == -1){
        result = true;
    }
    else{
        m->revealed[m->selectedY*10+m->selectedX] = 1;
        if(m->data[m->selectedY*10+m->selectedX] == 0){
            queue_t* que = create_queue(100);
            push_to_queue(que, m->selectedY*10+m->selectedX);
            while(get_queue_size(que) > 0){
                int idx = get_from_queue(que, 0);
                //if idx != -1
                int i = idx%10;
                int j = idx/10;
                if (i != 0){
                    reveal_part(m, que, (j)*10+i-1);
                }
                if (i != 9){
                    reveal_part(m, que, (j)*10+i+1);
                }
                if (j != 0){
                    reveal_part(m, que, (j-1)*10+i);
                }
                if (j != 9){
                    reveal_part(m, que, (j+1)*10+i);
                }
                if (i != 0 && j != 0){
                    reveal_part(m, que, (j-1)*10+i-1);
                }
                if (i != 0 && j != 9){
                    reveal_part(m, que, (j+1)*10+i-1);
                }
                if (i != 9 && j != 0){
                reveal_part(m, que, (j-1)*10+i+1);
                }
                if(i != 9 && j != 9){
                    reveal_part(m, que, (j+1)*10+i+1);
                }
                pop_from_queue(que);
            }
            delete_queue(que);
        }
    }
    return result;
}

void place_flag(mines *m){
    printf("Placed flag on %d, %d", m->selectedX, m->selectedY);
    fflush(stdout);
    if(m->revealed[((m->selectedY) * 10) + m->selectedX] == 0){
        m->revealed[((m->selectedY) * 10) + m->selectedX] = 2;
    }
    else if(m->revealed[((m->selectedY) * 10) + m->selectedX] == 2){
        m->revealed[((m->selectedY) * 10) + m->selectedX] = 0;
    }
}

bool checkWin(mines *m){
    int correct = 0;
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++){
            if(m->data[i*10+j] == -1 && m->revealed[i*10+j] == 2){
                correct++;
            }
            if(correct == 10){
                return true;
            }
        }
    }
    return false;
}

void drawField(mines *m, unsigned short *fb)
{
    /*drawes squares responsible to playfield*/
    for (int vert = 0; vert < 10; vert++)
    {
        for (int horz = 0; horz < 10; horz++)
        {
            if (m->revealed[vert * 10 + horz] != 0)
            {

                if (m->revealed[vert * 10 + horz] == 1)
                {
                    // shown
                    if (m->data[vert * 10 + horz] == -1)
                    {
                        // mine
                        draw_square(90 + (vert * 30), 20 + (horz * 30), 30, 30, 0x31, fb);
                    }
                    else
                    {
                        // number
                        draw_square(90 + (vert * 30), 20 + (horz * 30), 30, 30, 0x7ff, fb);
                        print_char((char)(m->data[vert * 10 + horz] + 48), 90 + (vert * 30) + 10, 20 + (horz * 30) + 10, 1, 0x0ff, fb);
                    }
                }
                else
                {
                    // draw flag
                    draw_square(90 + (vert * 30), 20 + (horz * 30), 30, 30, 0x70f, fb);
                    print_char((char)('F'), 90 + (vert * 30) + 10, 20 + (horz * 30) + 10, 1, 0x0ff, fb);
                }
            }
            else
            {
                // hidden
                draw_square(90 + (vert * 30), 20 + (horz * 30), 30, 30, 0x0f0, fb);
            }
            if (m->selectedX == horz && m->selectedY == vert)
            {
                // selected
                draw_hollow_square(90 + (vert * 30), 20 + (horz * 30), 30, 30, 2, 0x00, fb);
            }
        }
    }
}

void showScore(mines *m, unsigned short *fb)
{
    char text[20];
    int len = sprintf(text, "Score :%d", m->score);
    print_string(text, len, 200, 0, 1, 0x7ff, fb);
}
