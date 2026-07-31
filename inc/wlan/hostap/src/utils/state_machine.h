/*
 * wpa_supplicant/hostapd - ステートマシン定義
 * Copyright (c) 2002-2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This file includes a set of pre-processor macros that can be used to
 * implement a state machine. In addition to including this header file, each
 * file implementing a state machine must define STATE_MACHINE_DATA to be the
 * data structure including state variables (enum machine_state,
 * bool changed), and STATE_MACHINE_DEBUG_PREFIX to be a string that is used
 * as a prefix for all debug messages. If SM_ENTRY_MA macro is used to define
 * a group of state machines with shared data structure, STATE_MACHINE_ADDR
 * needs to be defined to point to the MAC address used in debug output.
 * SM_ENTRY_M macro can be used to define similar group of state machines
 * without this additional debug info.
 */

#ifndef INC_WLAN_UTILS_STATE_MACHINE_H
#define INC_WLAN_UTILS_STATE_MACHINE_H

#include <console.h>

/**
 * SM_STATE - ステートマシン関数の宣言
 * @machine: ステートマシン名
 * @state: ステートマシンの状態
 *
 * このマクロはステートマシン関数を宣言するために使用される。これは、C関数の
 * 定義の代わりに使用され、SM_ENTER / SM_ENTER_GLOBAL を呼び出すことで
 * そのステートに入った際に実行される関数の宣言に用いられる。
 */
#define SM_STATE(machine, state) \
static void sm_ ## machine ## _ ## state ## _Enter(STATE_MACHINE_DATA *sm, \
    int global)

/**
 * SM_ENTRY - ステートマシン関数エントリポイント
 * @machine: ステートマシン名
 * @state: ステートマシンの状態
 *
 * このマクロはSM_STATEで宣言されたステートマシン関数で使用される。
 * SM_ENTRYは関数本体の先頭で、ローカル変数の宣言の後に記述する必要がある。
 * このマクロは状態遷移に関するデバッグ情報を出力し、ステートマシンの状態を更新する。
 */
#define SM_ENTRY(machine, state) \
if (!global || sm->machine ## _state != machine ## _ ## state) { \
    sm->changed = true; \
    wpa_printf(MSG_DEBUG, STATE_MACHINE_DEBUG_PREFIX ": " #machine \
           " entering state " #state); \
} \
sm->machine ## _state = machine ## _ ## state;

/**
 * SM_ENTRY_M - ステートマシングループ用のステートマシン関数エントリポイント
 * @machine: ステートマシン名
 * @_state: ステートマシンの状態
 * @data: 状態変数prefix (full variable: prefix_state)
 *
 * このマクロはSM_ENTRYと同様であるが、複数のステートマシンで共有のデータ構造体を
 * 使用するステートマシングループ向けである。machine パラメータと prefix パラメータは
 * いずれも「サブステートマシン」名に設定される。prefix を使用することで、同一のデータ
 * 構造体に複数の状態変数を格納できるようになる。
 */
#define SM_ENTRY_M(machine, _state, data) \
if (!global || sm->data ## _ ## state != machine ## _ ## _state) { \
    sm->changed = true; \
    wpa_printf(MSG_DEBUG, STATE_MACHINE_DEBUG_PREFIX ": " \
           #machine " entering state " #_state); \
} \
sm->data ## _ ## state = machine ## _ ## _state;

/**
 * SM_ENTRY_MA - ステートマシングループ用のステートマシン関数エントリポイント
 * @machine: ステートマシン名
 * @_state: ステートマシンの状態
 * @data: 状態変数prefix (full variable: prefix_state)
 *
 * このマクロは SM_ENTRY_M と同様であるが、デバッグ出力に MAC アドレスが含まれる。
 * デバッグ出力に含める MAC アドレスを指すように、STATE_MACHINE_ADDR を定義する
 * 必要がある。
 */
#define SM_ENTRY_MA(machine, _state, data) \
if (!global || sm->data ## _ ## state != machine ## _ ## _state) { \
    sm->changed = true; \
    wpa_printf(MSG_DEBUG, STATE_MACHINE_DEBUG_PREFIX ": " MACSTR " " \
           #machine " entering state " #_state, \
           MAC2STR(STATE_MACHINE_ADDR)); \
} \
sm->data ## _ ## state = machine ## _ ## _state;

/**
 * SM_ENTER - 新しいマシンステートに入る
 * @machine: ステートマシン名
 * @state: ステートマシンの状態
 *
 * このマクロは、SM_STATE マクロで定義されたステートマシン関数への
 * 関数呼び出しに展開される。SM_ENTER は、ステートマシンのステップ関数内で
 * ステートマシンを新しい状態に移行させるために使用される。
 */
#define SM_ENTER(machine, state) \
sm_ ## machine ## _ ## state ## _Enter(sm, 0)

/**
 * SM_ENTER_GLOBAL - グローバル規則に基づいて新しいマシンステートに入る
 * @machine: ステートマシン名
 * @state: ステートマシンの状態
 *
 * このマクロは SM_ENTER と同様であるが、（特定の状態に限定されない）グローバルな
 * 規則に基づいて新しい状態に移行する際に使用される。同じグローバル規則により
 * ステートマシンが特定の状態に留まるよう強制されている場合に不要なデバッグメッセージが
 * 大量に表示されるのを防ぐため、別のマクロが使用される。
 */
#define SM_ENTER_GLOBAL(machine, state) \
sm_ ## machine ## _ ## state ## _Enter(sm, 1)

/**
 * SM_STEP - ステートマシンステップ関数の宣言
 * @machine: ステートマシン名
 *
 * このマクロはステートマシンのステップ関数を宣言するために使用される。
 * これは、C関数の定義の代わりに使用され、状態変数に基づいてステートマシンを
 * 新しい状態に移行させる関数を宣言する。この関数は、SM_ENTER マクロと
 * SM_ENTER_GLOBAL マクロを使用して新しい状態に移行する。
 */
#define SM_STEP(machine) \
static void sm_ ## machine ## _Step(STATE_MACHINE_DATA *sm)

/**
 * SM_STEP_RUN - ステートマシンステップ関数を呼び出す
 * @machine: ステートマシン名
 *
 * このマクロは　SM_STEP マクロで定義されたステートマシンのステップ関数への
 * 関数呼び出しに展開される。
 */
#define SM_STEP_RUN(machine) sm_ ## machine ## _Step(sm)

#endif /* STATE_MACHINE_H */
