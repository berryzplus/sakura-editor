﻿#!/usr/bin/env python
# -*- coding: utf-8 -*-

import pytest
import time

from appium import webdriver
from appium.options.windows import WindowsOptions
from appium.webdriver.appium_service import AppiumService
from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webdriver import WebDriver
from appium.webdriver.webelement import WebElement
from helper import appium_host as APPIUM_HOST
from helper import appium_port as APPIUM_PORT
from helper import appium_url as APPIUM_URL
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait
from typing import Generator

# appiumサーバーを起動する
@pytest.fixture(scope='session')
def appium_service():
    service = AppiumService()
    service.start(
        args=['--address', APPIUM_HOST, '--port', str(APPIUM_PORT)],
        timeout_ms=20000,
    )
    yield service
    service.stop()

@pytest.fixture(scope='session')
def root_driver(appium_service) -> Generator[WebDriver, None, None]:
    # build options
    # https://github.com/appium/appium-windows-driver
    # https://github.com/appium/python-client/blob/master/appium/options/windows/windows/app_top_level_window_option.py
    options = WindowsOptions()
    options.app = 'Root'
    root_driver = webdriver.Remote(APPIUM_URL, options=options)
    yield root_driver
    root_driver.quit()

@pytest.fixture(scope='session')
def desktop(root_driver: WebDriver) -> WebElement:
    return root_driver.find_element(By.CLASS_NAME, '#32769') # Desktop

@pytest.fixture(scope="function")
def tray_icon(desktop: WebElement) -> WebElement:
    # Windowsキー + B を押してタスクトレイをフォーカス
    desktop.send_keys(Keys.COMMAND, 'b')
    # Enterキーを押してタスクトレイを開く
    desktop.send_keys(Keys.ENTER)
    return WebDriverWait(desktop, 120).until(EC.visibility_of_element_located((By.XPATH, '//Button[contains(@Name, "サクラエディタ")]')))
