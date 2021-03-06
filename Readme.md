# perc (Эмиль Латыпов)
<h4>утилита для поиска папок и файлов, которые может изменить пользователь и группа</h4>
<hr>
Для компиляции утилиты возпользуйтесь

```bash
make build
```
 после чего программа будет скомпилирована и помещена в
 директорию bin/ корневого каталога проекта
 
для запуска утилиты, воспользуйтесь следующей коммандой
```bash
perc -u <username> -g <groupname> -p <path>
```
где <b>username</b> - имя пользователя для которого будет проверен доступ,
<b>groupname</b> - имя группы для которой будет проверен доступ,
<b>path</b> - путь для директории, которая будет просканирована
<br>
* Если путь до утилиты не был добавлен в переменную path,
тогда придется указывать полный путь до нее
<hr>
<h4>Тестирование</h4>
Для того, чтобы протестировать утилиту, предусмотрен
специальный сценарий. Чтобы запустить его, выполните команду:<br>

```bash
make test
```
Она сгенерирует тестовую папку и пользователей 
и запустит утилиту для каждого из них
<hr>
Для того чтобы удалить временные файлы/пользователей, используйте

```bash
make clean
```