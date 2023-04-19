# バージョン指定
cmake_minimum_required(VERSION 3.15)

# プロジェクト設定
project(AUDIO_PLAYER
LANGUAGES CXX
VERSION 0.0.1
)

# サブディレクトリを追加
add_subdirectory(libs/JUCE)

add_subdirectory(src)
