Synchronous Reliable Broadcast
====================

| Название | sync-rb          |
| -------- | ---------------- |
| Баллы    | 5                |
| Дедлайн  | 01.12.2021 09:00 |


# Подготовка

*Синхронная* распределённая *система* &mdash; система, в которой все сообщения доставляются и обрабатываются за время, не превышающее наперёд известное Δ.

```pseudocode
send m to all processes
wait delta
// by now all correct processes received m
```

Если процесс отправил сообщение и упал раньше, чем прошло Δ, то сообщение могло как дойти, так и потеряться.

```pseudocode
send m to all processes
// assume the sender crashed right after invocation of send
// some processes received m while others didn't
```



*Message complexity* &mdash; общее количество unicast-сообщений, отправляемых в процессе работы алгоритма.

Под *optimistic message complexity* в этом задании будем подразумевать общее количество unicast-сообщений при условии, что все процессы корректные.

*Broadcast* &mdash; это примитив, позволяющий одному источнику разослать сообщение группе процессов. Задачи *reliable broadcast* и *uniform reliable broadcast* определяются свойствами, которые можно найти [в презентации](https://docs.google.com/presentation/d/1BH87IFXdjmZZqr0oqFAH3vbcsoCmiZ_QZjNi8DSZsrA/edit?usp=sharing). Там же есть диаграммы алгоритмов, реализующих RB и URB в асинхронной системе.

Мы будем оценивать message complexity асимптотически. Так, message complexity асинхронного reliable broadcast'а O(n^2), где n &mdash; количество процессов.

# Задание

1. Придумайте алгоритм reliable broadcast для синхронной системы с optimistic message complexity O(n).
   
   За этот пункт вы получите 3 балла.

2. Доработайте алгоритм из пункта 1, чтобы получился uniform reliable broadcast. Optimistic message complexity должно остаться O(n). Сравните отказоустойчивость получившегося алгоритма с отказоустойчивостью асинхронного uniform reliable broadcast.
   
   За этот пункт вы получите ещё 2 балла.

# Сдача

В этом задании вам нужно описать алгоритмы текстом или псевдокодом. Решения принимаются в форматах pdf и markdown.

Положите файл `SOLUTION.md` или `SOLUTION.pdf` в папку `sync-rb`, запушьте в ветку `sync-rb` и откройте pull-request в `master`.