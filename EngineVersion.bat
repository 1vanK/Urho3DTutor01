:: Указываем путь к git.exe.
set "PATH=c:\Program Files (x86)\Git\bin\"
:: Скачиваем репозиторий.
git clone https://github.com/Urho3D/Urho3D.git
:: Переходим в папку со скачанными исходниками.
cd Urho3D
:: Возвращаем состояние репозитория к определённой версии (20 февраля 2017).
git reset --hard 7451d6feceb303d3fe43663057aaa05418ac642f
:: Ждём нажатия ENTER для закрытия консоли.
pause
