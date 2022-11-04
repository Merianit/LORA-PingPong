/*
 * Keypad.h
 *
 *  Created on: Sep 5, 2021
 *      Author: Тлехас Алий
 *
 *	
 *	Based on https://github.com/Chris--A/Keypad
 */

#ifndef KEYBOARD_KEYPAD_INC_KEYPAD_H_
#define KEYBOARD_KEYPAD_INC_KEYPAD_H_

#include "main.h"
#include "key.h"
#include "stdbool.h"

typedef char KeypadEvent;

#define OPEN GPIO_PIN_RESET
#define CLOSED GPIO_PIN_SET

#define LIST_MAX 10		// Максимальное количество активных кнопок
#define MAPSIZE 6		// MAPSIZE - это количество строк (до 16 символов в строке)
#define makeKeymap(x) ((char*)x)

// струтура-обертка для работы с пинами, подключенными к матричной клавиатуре
typedef struct {
	int pin;
	GPIO_TypeDef *port;
} Keypad_dio_t;

// структура-обертка для хранения размеров клавиатуры
typedef struct {
    uint8_t rows;
    uint8_t columns;
} KeypadSize;

// основная структура для работы с клавиатурой и ее инициализации
typedef struct {
	uint16_t bitMap[MAPSIZE];
	Key_t key[LIST_MAX];
	uint32_t holdTimer;
} Keypad_t;


// функция инициализации клавиатуры, возвращает ноль, если уже проинициализирована, не более одной активной клавиатуры
_Bool Keypad_create(Keypad_t *keypad, char *userKeymap, const Keypad_dio_t *row, const Keypad_dio_t *col, uint8_t numRows, uint8_t numCols);


// получение символа первой нажатой кнопки
// также является основной функцией для работы клавиатуры, которая должна вызываться периодически
char Keypad_getKey(Keypad_t *keypad);

// Блокирующая функция, ожидающая нажатие какой-либо из кнопок
// *прим: не рекомендуется к использованию
char Keypad_waitForKey(Keypad_t *keypad);

// получение статуса о том, есть ли изменение в состоянии какой-нибудь из кнопок
_Bool Keypad_getKeys(Keypad_t *keypad);

// Возвращает состояние любой из нажатых кнопок
KeyState Keypad_getState(Keypad_t *keypad);

// Если кнопка определенного символа keyChar нажата, то возвращает true
_Bool Keypad_isPressed(Keypad_t *keypad, char keyChar);

// Установки задержки для антидребезга 
void Keypad_setDebounceTime(uint16_t debounce);

//Установить время удержания
void Keypad_setHoldTime(uint16_t time);

// Инициализировать событие, если есть какие-нибудь изменения в состоянии клавиатуры
void Keypad_addEventListener(void (*listener)(char));

// Возвращает единицу, если есть измение в состоянии в одной из кнопок (только [0] в отличие от getKeys)
_Bool Keypad_keyStateChanged(Keypad_t *keypad);

// Поиск по символу в списке обрабытываемых кнопок
int Keypad_findInListChar(Keypad_t *keypad, char keyChar);

// Поиск по номеру в списке обрабатываемых кнопок
int Keypad_findInListInt(Keypad_t *keypad, int keyCode);

// Функция, возвращаяющая количество кнопок в клавиатуре
uint8_t Keypad_numKeys(Keypad_t *keypad);


#endif /* KEYBOARD_KEYPAD_INC_KEYPAD_H_ */
