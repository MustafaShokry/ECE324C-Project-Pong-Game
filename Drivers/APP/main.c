#include "../LIB/BIT_MATH.h"
#include "../LIB/STD_TYPES.h"

#include "../tm4c123gh6pm.h"
#include "../MCAL/NVIC/NVIC_int.h"
#include "../MCAL/GPIO/GPIO.h"
#include "../MCAL/GPIO/GPIO_Cfg.h"
#include "../MCAL/GPTM/GPTM_Config.h"
#include "../MCAL/GPTM/GPTM_Interface.h"
#include "../MCAL/SSI/SSI_int.h"
#include "../MCAL/UART/UART_Config.h"
#include "../MCAL/UART/UART_Interface.h"
#include "../HAL/N5110/N5110_int.h"

#include "main.h"

extern GPTM_Config_t GPTM_Config;
extern UART_Config_t UART_Config;

u8 global_u8ElapsedTimeBeriod = 0;
u8 global_u8UARTBuffer = 0;

int main(void){
	GPIO_Init();
			
	HN5110_vInit();
    UART_VoidInit(&UART_Config);
    UART_VoidSetUARTInterruptCallBack(UART_0, APP_voidUARTRQ);
    GPTM_VoidInit(&GPTM_Config);
	GPTM_VoidSetTimeOutRawInterruptCallBack(TIMER_2, COUNTER_A, APP_voidRefreshIRQ);
	MNVIC_vEnableINTPeripheral(NVIC_TIMER2A);
	MNVIC_vEnableINTPeripheral(NVIC_UART0);
    while (1)
    {
        APP_voidDisplayOpening();
        APP_voidPlay();
    }
    
}

void APP_voidDisplayOpening(void){
    PADDLE_t local_PaddleOne, local_PaddleTwo;
    BALL_t local_Ball;
    u8 local_u8Itterator = 0;

    local_PaddleOne.xPos = 0;
    local_PaddleOne.yPos = (N5110_SCREENH / 2) - (PLAYERHIGHT / 2);
    local_PaddleOne.speed = PADDLE_SPEED_START;

    local_PaddleTwo.xPos = N5110_SCREENW - PLAYERWIDTH;
    local_PaddleTwo.yPos = (N5110_SCREENH / 2) - (PLAYERHIGHT / 2);
    local_PaddleTwo.speed = PADDLE_SPEED_START;

    local_Ball.xPos = (N5110_SCREENW / 2);
    local_Ball.yPos = (N5110_SCREENH / 2);
    local_Ball.xSpeed = 1;
    local_Ball.ySpeed = 1;


    while (1)
    {
        if(global_u8ElapsedTimeBeriod == 1){
            global_u8ElapsedTimeBeriod = 0;
            HN5110_vClearBuffer();
            APP_voidDrawFillCircle(local_Ball.xPos, local_Ball.yPos, BALLRADIUS);
            APP_voidDrawVLine(N5110_SCREENW / 2, 0, N5110_SCREENH - 1);
            APP_voidDrawFillRect(local_PaddleOne.xPos, local_PaddleOne.yPos, PLAYERWIDTH, PLAYERHIGHT);
            APP_voidDrawFillRect(local_PaddleTwo.xPos, local_PaddleTwo.yPos, PLAYERWIDTH, PLAYERHIGHT);
            HN5110_vAddStringBuffer((N5110_SCREENW - 7 * 8) / 2, 15, "Press x");
            

            if(local_u8Itterator <= 15){
                HN5110_vAddStringBuffer((N5110_SCREENW - 4 * 8) / 2, 25, "Play");
                local_u8Itterator++;
            }else{
                local_u8Itterator++;
                if(local_u8Itterator >= 30) local_u8Itterator = 0;
            }
            HN5110_vDisplayBuffer();

            if(local_Ball.xPos <= N5110_SCREENW / 2){
                if(local_Ball.yPos > local_PaddleOne.yPos + PLAYERHIGHT / 2){
                    if(local_PaddleOne.yPos + PLAYERHIGHT < N5110_SCREENH) local_PaddleOne.yPos += local_PaddleOne.speed;
                }else if(local_Ball.yPos < local_PaddleOne.yPos + PLAYERHIGHT / 2){
                    if(local_PaddleOne.yPos > 0) local_PaddleOne.yPos -= local_PaddleOne.speed;
                }
            }

            if(local_Ball.xPos >= N5110_SCREENW / 2){
                if(local_Ball.yPos > local_PaddleTwo.yPos + PLAYERHIGHT / 2){
                    if(local_PaddleTwo.yPos + PLAYERHIGHT < N5110_SCREENH) local_PaddleTwo.yPos += local_PaddleTwo.speed;
                }else if(local_Ball.yPos < local_PaddleTwo.yPos + PLAYERHIGHT / 2){
                    if(local_PaddleTwo.yPos > 0) local_PaddleTwo.yPos -= local_PaddleTwo.speed;
                }
            }

            if(APP_u8CheckBallRectCollide(&local_Ball, &local_PaddleOne) || APP_u8CheckBallRectCollide(&local_Ball, &local_PaddleTwo)){
                local_Ball.xSpeed *= -1;
            }

            if(local_Ball.yPos + BALLRADIUS >= N5110_SCREENH - 1 || local_Ball.yPos - BALLRADIUS <= 1) local_Ball.ySpeed *= -1;

            local_Ball.xPos += local_Ball.xSpeed;
            local_Ball.yPos += local_Ball.ySpeed;

            if(local_Ball.xPos - BALLRADIUS <= 0){
                local_Ball.xPos = (N5110_SCREENW / 2);
                local_Ball.yPos = (N5110_SCREENH / 2);
                local_Ball.xSpeed = 1;
                local_Ball.ySpeed = 1;
            }else if(local_Ball.xPos + BALLRADIUS >= N5110_SCREENW){
                local_Ball.xPos = (N5110_SCREENW / 2);
                local_Ball.yPos = (N5110_SCREENH / 2);
                local_Ball.xSpeed = 1;
                local_Ball.ySpeed = 1;
            }

            if(GPIO_ReadPin(OK_PIN_PORT, OK_PIN_PIN) == GPIO_LOW){
                break;
            }
        }
    }
}

void APP_voidPlay(void){
    PADDLE_t local_PaddleOne, local_PaddleTwo;
    BALL_t local_Ball;
    u8 local_u8Player1Score = 0, local_u8Player2Score = 0;
    u8 local_u8Player2Input = 0;

    local_PaddleOne.xPos = 0;
    local_PaddleOne.yPos = (N5110_SCREENH / 2) - (PLAYERHIGHT / 2);
    local_PaddleOne.speed = PADDLE_SPEED_START;

    local_PaddleTwo.xPos = N5110_SCREENW - PLAYERWIDTH;
    local_PaddleTwo.yPos = (N5110_SCREENH / 2) - (PLAYERHIGHT / 2);
    local_PaddleTwo.speed = PADDLE_SPEED_START;

    local_Ball.xPos = (N5110_SCREENW / 2);
    local_Ball.yPos = (N5110_SCREENH / 2);
    local_Ball.xSpeed = BALL_X_SPEED_START;
    local_Ball.ySpeed = BALL_Y_SPEED_START;
	  
    GPTM_VoidSetTimerReloadValue(TIMER_2, COUNTER_A, REFRESH_RATE_PERIODE);
    while (1)
    {
        if(global_u8ElapsedTimeBeriod == 1){
            global_u8ElapsedTimeBeriod = 0;
            GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_LOW);
            HN5110_vClearBuffer();
            APP_voidDrawFillCircle(local_Ball.xPos, local_Ball.yPos, BALLRADIUS);
            APP_voidDrawVLine(N5110_SCREENW / 2, 0, N5110_SCREENH - 1);
            APP_voidDrawFillRect(local_PaddleOne.xPos, local_PaddleOne.yPos, PLAYERWIDTH, PLAYERHIGHT);
            APP_voidDrawFillRect(local_PaddleTwo.xPos, local_PaddleTwo.yPos, PLAYERWIDTH, PLAYERHIGHT);
            HN5110_vAddNumberBuffer((N5110_SCREENW / 4) - APP_u8GetNumLength(local_u8Player1Score) * 4, 0, local_u8Player1Score);
            HN5110_vAddNumberBuffer((N5110_SCREENW * 3 / 4) - APP_u8GetNumLength(local_u8Player2Score) * 4 , 0, local_u8Player2Score);

            HN5110_vDisplayBuffer();

            if(local_u8Player2Score == 15 || local_u8Player1Score == 15){
                break;
            }
            if(GPIO_ReadPin(UP_PIN_PORT, UP_PIN_PIN) == GPIO_LOW){
                if(local_PaddleOne.yPos > 0) local_PaddleOne.yPos -= local_PaddleOne.speed;
            }else if(GPIO_ReadPin(DOWN_PIN_PORT, DOWN_PIN_PIN) == GPIO_LOW){
                if(local_PaddleOne.yPos + PLAYERHIGHT < N5110_SCREENH) local_PaddleOne.yPos += local_PaddleOne.speed;
            }

            //Player 2
						
            local_u8Player2Input = global_u8UARTBuffer;
            if(local_u8Player2Input == 'w' || local_u8Player2Input == 'W'){
                if(local_PaddleTwo.yPos > 0) local_PaddleTwo.yPos -= local_PaddleTwo.speed;
							global_u8UARTBuffer = 0;
            }else if(local_u8Player2Input == 's' || local_u8Player2Input == 'S'){
                if(local_PaddleTwo.yPos + PLAYERHIGHT < N5110_SCREENH) local_PaddleTwo.yPos += local_PaddleTwo.speed;
							global_u8UARTBuffer = 0;
            }

            local_u8Player2Input = 0;

            if(APP_u8CheckBallRectCollide(&local_Ball, &local_PaddleOne) || APP_u8CheckBallRectCollide(&local_Ball, &local_PaddleTwo)){
                local_Ball.xSpeed *= -1;
                GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_HIGH);
            }

            if(local_Ball.yPos + BALLRADIUS >= N5110_SCREENH - 1 || local_Ball.yPos - BALLRADIUS <= 1){
                local_Ball.ySpeed *= -1;
                GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_HIGH);
            }

            local_Ball.xPos += local_Ball.xSpeed;
            local_Ball.yPos += local_Ball.ySpeed;

            if(local_Ball.xPos - BALLRADIUS <= 0){
                local_u8Player2Score++;
                local_Ball.xPos = (N5110_SCREENW / 2);
                local_Ball.yPos = (N5110_SCREENH / 2);
                local_Ball.xSpeed = BALL_X_SPEED_START;
                local_Ball.ySpeed = BALL_Y_SPEED_START;
            }else if(local_Ball.xPos + BALLRADIUS >= N5110_SCREENW){
                local_u8Player1Score++;
                local_Ball.xPos = (N5110_SCREENW / 2);
                local_Ball.yPos = (N5110_SCREENH / 2);
                local_Ball.xSpeed = BALL_X_SPEED_START;
                local_Ball.ySpeed = BALL_Y_SPEED_START;
            }
        }
    }

    if(local_u8Player1Score > local_u8Player2Score){
        HN5110_vAddStringBuffer((N5110_SCREENW - 8 * 8) / 2, 15, "Player 1");
        
    }else{
        HN5110_vAddStringBuffer((N5110_SCREENW - 8 * 8) / 2, 15, "Player 2");
    }
    HN5110_vAddStringBuffer((N5110_SCREENW - 4 * 8) / 2, 25, "Wins");
    HN5110_vDisplayBuffer();
    while (GPIO_ReadPin(OK_PIN_PORT, OK_PIN_PIN) == GPIO_HIGH);
}

void APP_voidDrawFillRect(s16 A_s16XPos, s16 A_s16YPos, u8 A_u8Width, u8 A_u8Hight){
    u8 local_u8Itterator = 0;

    for(local_u8Itterator = A_s16XPos; local_u8Itterator < A_s16XPos + A_u8Width; ++local_u8Itterator){
        APP_voidDrawVLine(local_u8Itterator, A_s16YPos, A_s16YPos + A_u8Hight);
    }

}


void APP_voidDrawFillCircle(s16 A_s16XPos, s16 A_s16YPos, s16 A_s16Radius){
    s16 local_s16pk = 1 - A_s16Radius;
    s16 local_s16XPos = 0, local_s16YPos = A_s16Radius;

    while (local_s16XPos <= local_s16YPos)
    {
        APP_voidDrawVLine(A_s16XPos + local_s16XPos, A_s16YPos - local_s16YPos, A_s16YPos + local_s16YPos);
        APP_voidDrawVLine(A_s16XPos + local_s16YPos, A_s16YPos - local_s16XPos, A_s16YPos + local_s16XPos);
        APP_voidDrawVLine(A_s16XPos - local_s16XPos, A_s16YPos - local_s16YPos, A_s16YPos + local_s16YPos);
        APP_voidDrawVLine(A_s16XPos - local_s16YPos, A_s16YPos - local_s16XPos, A_s16YPos + local_s16XPos);

        ++local_s16XPos;

        if(local_s16pk <= 0){
            local_s16pk += 2 * local_s16XPos + 3;
        }else{
            --local_s16YPos;

            local_s16pk += 2 * (local_s16XPos - local_s16YPos) + 5;
        }
    }
}

void APP_voidDrawCircle(s16 A_s16XPos, s16 A_s16YPos, s16 A_s16Radius){
    s16 local_s16pk = 1 - A_s16Radius;
    s16 local_s16XPos = 0, local_s16YPos = A_s16Radius;

    while (local_s16XPos <= local_s16YPos)
    {

        HN5110_vSetPixel(A_s16XPos + local_s16XPos, A_s16YPos - local_s16YPos, N5110_HIGH);
        HN5110_vSetPixel(A_s16XPos + local_s16XPos, A_s16YPos + local_s16YPos, N5110_HIGH);

        HN5110_vSetPixel(A_s16XPos - local_s16XPos, A_s16YPos - local_s16YPos, N5110_HIGH);
        HN5110_vSetPixel(A_s16XPos - local_s16XPos, A_s16YPos + local_s16YPos, N5110_HIGH);

        HN5110_vSetPixel(A_s16XPos - local_s16YPos, A_s16YPos + local_s16XPos, N5110_HIGH);
        HN5110_vSetPixel(A_s16XPos + local_s16YPos, A_s16YPos + local_s16XPos, N5110_HIGH);

        HN5110_vSetPixel(A_s16XPos - local_s16YPos, A_s16YPos - local_s16XPos, N5110_HIGH);
        HN5110_vSetPixel(A_s16XPos + local_s16YPos, A_s16YPos - local_s16XPos, N5110_HIGH);

        ++local_s16XPos;

        if(local_s16pk <= 0){
            local_s16pk += 2 * local_s16XPos + 3;
        }else{
            --local_s16YPos;

            local_s16pk += 2 * (local_s16XPos - local_s16YPos) + 5;
        }
    }
}

void APP_voidDrawVLine(s16 A_s16XPos, s16 A_s16Y1Pos, s16 A_s16Y2Pos){
    s16 local_s16Iterrator = 0;
    if(A_s16Y1Pos <= A_s16Y2Pos)
    for(local_s16Iterrator = A_s16Y1Pos; local_s16Iterrator <= A_s16Y2Pos; ++local_s16Iterrator){
        if(A_s16XPos >= 0 && A_s16XPos < N5110_SCREENW && local_s16Iterrator >= 0 && local_s16Iterrator < N5110_SCREENH)
            HN5110_vSetPixel(A_s16XPos, local_s16Iterrator, N5110_HIGH);
    }
}

u8 APP_u8CheckBallRectCollide(BALL_t* A_pBALL, PADDLE_t *A_pPaddle){
    u8 local_u8BallAbove = 0, local_u8BallBelow = 0, local_u8BallLeft = 0, local_u8BallRight = 0;
    u8 local_u8CollideHappen = 0;
    local_u8BallAbove = (A_pBALL->yPos < A_pPaddle->yPos);
    local_u8BallBelow = (A_pBALL->yPos > A_pPaddle->yPos + PLAYERHIGHT);
    local_u8BallLeft = (A_pBALL->xPos <= A_pPaddle->xPos);
    local_u8BallRight = (A_pBALL->xPos >= A_pPaddle->xPos + PLAYERWIDTH);

    if(local_u8BallAbove && local_u8BallLeft){

        local_u8CollideHappen = (BALLRADIUS * BALLRADIUS >= ((A_pPaddle->xPos - A_pBALL->xPos) * (A_pPaddle->xPos - A_pBALL->xPos) + (A_pPaddle->yPos - A_pBALL->yPos) * (A_pPaddle->yPos - A_pBALL->yPos)));

    }else if(local_u8BallAbove && local_u8BallRight){

        local_u8CollideHappen = (BALLRADIUS * BALLRADIUS >= ((A_pPaddle->xPos + PLAYERWIDTH - A_pBALL->xPos) * (A_pPaddle->xPos + PLAYERWIDTH - A_pBALL->xPos) + (A_pPaddle->yPos - A_pBALL->yPos) * (A_pPaddle->yPos - A_pBALL->yPos)));
    
    }else if(local_u8BallBelow && local_u8BallLeft){
    
        local_u8CollideHappen = (BALLRADIUS * BALLRADIUS >= ((A_pPaddle->xPos - A_pBALL->xPos) * (A_pPaddle->xPos - A_pBALL->xPos) + (A_pPaddle->yPos + PLAYERHIGHT - A_pBALL->yPos) * (A_pPaddle->yPos + PLAYERHIGHT - A_pBALL->yPos)));
    
    }else if(local_u8BallBelow && local_u8BallRight){

        local_u8CollideHappen = (BALLRADIUS * BALLRADIUS >= ((A_pPaddle->xPos + PLAYERWIDTH - A_pBALL->xPos) * (A_pPaddle->xPos + PLAYERWIDTH - A_pBALL->xPos) + (A_pPaddle->yPos + PLAYERHIGHT - A_pBALL->yPos) * (A_pPaddle->yPos + PLAYERHIGHT - A_pBALL->yPos)));

    }else if(local_u8BallLeft){

        local_u8CollideHappen = (BALLRADIUS >= A_pPaddle->xPos - A_pBALL->xPos);

    }else if(local_u8BallRight){

        local_u8CollideHappen = (BALLRADIUS >= A_pBALL->xPos - (A_pPaddle->xPos + PLAYERWIDTH));

    }else{
        local_u8CollideHappen = 0;
    }

    return local_u8CollideHappen;
}

void APP_voidRefreshIRQ(void){
    global_u8ElapsedTimeBeriod = 1;
}

u8 APP_u8GetNumLength(u8 A_u8Num){
    u8 A_u8Result = 0;
    if(A_u8Num == 0){
        A_u8Result = 1;
    }else {
        while (A_u8Num != 0)
        {
            A_u8Result++;
            A_u8Num /= 10;
        }
    }
    return A_u8Result;
    
}

void APP_voidUARTRQ(void){
    global_u8UARTBuffer = UART_UnsignedCharGetDataNonBlocking(UART_0);
}
