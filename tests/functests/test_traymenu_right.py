#!/usr/bin/env python
# -*- coding: utf-8 -*-
import ctypes
import time

from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webdriver import WebDriver
from appium.webdriver.webelement import WebElement
from helper import wait_for_context_menu
from helper import wait_for_dialog
from helper import cancel_element
from selenium.webdriver import ActionChains
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait
from typing import Final

def context_click(root_driver, tray_icon):
    rect = tray_icon.rect
    element_center_x = rect['x'] + rect['width'] // 2
    element_center_y = rect['y'] + rect['height'] // 2

    # マルチディスプレイの情報を取得するための設定
    user32 = ctypes.windll.user32
    user32.SetProcessDPIAware()

    # プライマリディスプレイの左上座標を取得
    primary_screen_left = user32.GetSystemMetrics(76)  # SM_XVIRTUALSCREEN: プライマリディスプレイの左上X座標
    primary_screen_top = user32.GetSystemMetrics(77)  # SM_YVIRTUALSCREEN: プライマリディスプレイの左上Y座標

    element_abs_x = element_center_x - primary_screen_left
    element_abs_y = element_center_y - primary_screen_top

    # 右クリックでポップアップメニューを表示
    # https://github.com/appium/appium-windows-driver#windows-click
    root_driver.execute_script('windows: click', {
        'x': element_abs_x,
        'y': element_abs_y,
        'button': 'right',
        })

def test_show_type_list(root_driver, desktop, tray_icon):
    context_click(root_driver, tray_icon)

    # メニュー項目F_TYPE_LISTをクリック
    F_TYPE_LIST: Final = '31110'
    context_menu = wait_for_context_menu(desktop,  F_TYPE_LIST)
    context_menu.click()
    WebDriverWait(desktop, 120).until_not(EC.visibility_of_element_located((By.ID, context_menu.id)))

    # 「タイプ別設定一覧」ダイアログが表示されるまで待つ
    type_list = WebDriverWait(root_driver, 120).until(EC.presence_of_element_located((By.CLASS_NAME, '#32770')))

    # キャンセルで閉じる
    cancel_element(desktop, type_list)

def test_show_common_prop(root_driver, desktop, tray_icon):
    context_click(root_driver, tray_icon)

    # メニュー項目F_OPTIONをクリック
    F_OPTION: Final = '31112'
    context_menu = wait_for_context_menu(desktop,  F_OPTION)
    context_menu.click()
    WebDriverWait(desktop, 120).until_not(EC.visibility_of_element_located((By.ID, context_menu.id)))

    # 「共通設定」ダイアログが表示されるまで待つ
    common_prop = wait_for_dialog(desktop, '共通設定')

    # キャンセルで閉じる
    cancel_element(desktop, common_prop)

def test_show_about_dialog(root_driver, desktop, tray_icon):
    context_click(root_driver, tray_icon)

    # メニュー項目F_ABOUTをクリック
    F_ABOUT: Final = '31455'
    context_menu = wait_for_context_menu(desktop,  F_ABOUT)
    context_menu.click()
    WebDriverWait(desktop, 120).until_not(EC.visibility_of_element_located((By.ID, context_menu.id)))

    # 「バージョン情報」ダイアログが表示されるまで待つ
    about_dialog = wait_for_dialog(desktop, 'バージョン情報')

    # キャンセルで閉じる
    cancel_element(desktop, about_dialog)
