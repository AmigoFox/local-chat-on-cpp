function Controller() {
    // Показываем стандартные страницы
    installer.setDefaultPageVisible(QInstaller.Introduction, true);
    installer.setDefaultPageVisible(QInstaller.LicenseCheck, true);
    installer.setDefaultPageVisible(QInstaller.TargetDirectory, true);
    installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, true);
    installer.setDefaultPageVisible(QInstaller.Finished, true);
}

function Component() {}

Component.prototype.createOperations = function() {
    // Создаём стандартные операции установки
    component.createOperations();

    // === Параметры ===
    var appName = "Local messages"; // имя ярлыка
    var targetPath = "@TargetDir@/untitled.exe"; // путь к exe
    var iconPath = "@TargetDir@/installer/images/6172.ico"; // путь к иконке

    // === Создание ярлыка на рабочем столе ===
    component.addOperation("CreateShortcut",
        targetPath,
        "@DesktopDir@/" + appName + ".lnk",
        "workingDirectory=@TargetDir@",
        "iconPath=" + iconPath);

    // === Создание ярлыка в меню Пуск ===
    component.addOperation("CreateShortcut",
        targetPath,
        "@StartMenuDir@/" + appName + ".lnk",
        "workingDirectory=@TargetDir@",
        "iconPath=" + iconPath);
};
