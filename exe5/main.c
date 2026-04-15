/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueButId; 
QueueHandle_t xQueueBtn2;

SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_y;


int delay_red = 0;
int delay_yel = 0;

void btn_callback(uint gpio, uint32_t events) {
    if (gpio == BTN_PIN_R)
        xQueueSend(xQueueButId, &gpio, 0); // manda qual botão foi apertado
    else if (gpio == BTN_PIN_Y)
        xQueueSend(xQueueBtn2, &gpio, 0);
}

void btn_1_task(void* p) {
    uint gpio = 0; // adiciona no início da função
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);
    while (true) {
        if (xQueueReceive(xQueueButId, &gpio, 0)){
            if (gpio == BTN_PIN_R) {
                delay_red += 100;
            }
            if (delay_red > 1000) delay_red = 100;
            xSemaphoreGive(xSemaphore_r);
        }
        else if (xQueueReceive(xQueueBtn2, &gpio, 0)){
            if (gpio == BTN_PIN_Y) {
                delay_yel += 100;
            }
            if (delay_yel > 1000) delay_yel = 100;
            xSemaphoreGive(xSemaphore_y);
        }
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    while (true) {
        if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE){
            if (delay_red > 0) {
                gpio_put(LED_PIN_R, 1);
                vTaskDelay(pdMS_TO_TICKS(delay_red));
                gpio_put(LED_PIN_R, 0);
                vTaskDelay(pdMS_TO_TICKS(delay_red));
            }
        }         
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    while (true) {
        if (xSemaphoreTake(xSemaphore_y, pdMS_TO_TICKS(500)) == pdTRUE){
            if (delay_yel > 0) {
                gpio_put(LED_PIN_Y, 1);
                vTaskDelay(pdMS_TO_TICKS(delay_yel));
                gpio_put(LED_PIN_Y, 0);
                vTaskDelay(pdMS_TO_TICKS(delay_yel));
            }
        }         
    }
}


int main() {
    stdio_init_all();
    xQueueButId = xQueueCreate(32, sizeof(int));
    xQueueBtn2 = xQueueCreate(32, sizeof(int));
    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_y = xSemaphoreCreateBinary();

    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while(1){}

    return 0;
}




