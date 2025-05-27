<div align="center">
  <picture>
    <img src="./static/images/bao_lang.png">
  </picture>

  Bao - Ngôn ngữ lập trình Tiếng Việt

  ![Status](https://img.shields.io/badge/status-WIP-orange) ![Powered by LLVM](https://img.shields.io/badge/powered%20by-LLVM-darkgreen) ![GitHub stars](https://img.shields.io/github/stars/bao-langu/bao?style=social)
</div>

# Ngôn ngữ lập trình Bao
Xin chào đến với dự án trình biên dịch Bao!

Repo này chứa mã nguồn của trình biên dịch. Mục đích cuối cùng của dự án này là có thể "Bao" bọc được các thư viện chương trình C và C++ vào một ngôn ngữ lập trình dễ hơn. Ngôn ngữ mang tính chất giáo dục là chính.

# Tại sao Tiếng Việt?
- Bởi không có ngôn ngữ lập trình nào (ít nhất thì tôi không biết) mà sử dụng Tiếng Việt cả. Và dự án này sẽ giúp hiểu biết được thêm cách xử lý các ký tự Unicode trong một trình biên dịch và từ khoá nhiều từ.

- Nói chung là ngôn ngữ này có mục đích nghiên cứu và học tập chứ không phải để dùng thường ngày.

# Build
![GitHub release](https://img.shields.io/github/v/release/bao-langu/bao) ![Platform](https://img.shields.io/badge/platform-linux%20%7C%20macOS%20%7C%20windows-blue)

Hướng dẫn build cho máy của mình
### macOS và Linux

Đầu tiên là cài LLVM (backend của trình biên dịch)
- Đối với macOS bạn hãy cài [Homebrew](https://brew.sh)
rồi chạy câu lệnh này
```
brew install llvm
```
- Đối với Linux thì chạy câu lệnh này
- (Mới thử trên ubuntu/debian, có thể phải resolve nhiều thứ nếu không cẩn thận)
```
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 20
sudo apt install llvm-20
```
Còn lại thì cả hai đều chạy câu lệnh sau trong terminal
```
./unix-setup.sh
cmake --build build
```
### Windows
Trình biên dịch Bao cần [Visual Studio Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/). Làm ơn cài đặt trước khi tiến trình

Đầu tiên bạn cài LLVM. Bạn clone từ [đây](https://github.com/llvm/llvm-project) rồi thực hiện các câu lệnh sau trong thư mục dự án
```
mkdir build
cd build
cmake -DLLVM_ENABLE_PROJECTS="" -DLLVM_TARGETS_TO_BUILD="X86" -DCMAKE_BUILD_TYPE=Release ..\llvm
cmake --build . --config Release
cmake --build . --config Debug
```

Xong bạn copy thư mục `lib\cmake\llvm` vào thư mục `extern\llvm` của dự án Bao.

Còn lại thì chạy câu lệnh sau trong PowerShell
```
.\windows-setup.ps1
cmake --build build
```

> [!NOTE]
> Để sử dụng trình biên dịch bạn phải khai báo LIB trong System environment variables cho các thư viện hệ thống sau: libcmt.lib, libucrt.lib, kernel32.lib, user32.lib

# Đóng Góp
![Contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen) ![GitHub issues](https://img.shields.io/github/issues/bao-langu/bao)

Làm ơn đóng góp vào dự án nếu bạn tìm thấy bug , điểm yếu, hoặc để lại bình luận.

# License
![License](https://img.shields.io/github/license/bao-langu/bao)

Dự án Bao nằm dưới giấy phép Apache-2.0. Đọc thêm tại đây: [LICENSE-APACHE](LICENSE)

Phần mềm này đi kèm với bộ phận của dự án LLVM mà nằm dưới giấy phép Apache License 2.0 with LLVM Exceptions. Đọc thêm tại đây: [LICENSE-LLVM](LICENSE-LLVM.txt)