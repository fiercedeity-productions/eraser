#include "main.h"
#include "lib.h"
#include "standards.h"
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <wx/wx.h>

bool App::OnInit() {
	// static std::mutex mut;
	// try {
	// 	GoodBye::overwriteBytesMultiple("hello", 1024 * 512, standards::GUTMANN,
	// 	                               [&](size_t progress, size_t fileSize, size_t pass) -> void {
	// 		                               std::lock_guard<std::mutex> lock(mut);
	// 		                               std::cout << "pass:\t" << pass << "\tbytes written:\t" << progress
	// 		                                         << "\ttotal bytes:\t" << fileSize << "\tprogress:\t"
	// 		                                         << static_cast<double>(100) * static_cast<double>(progress) /
	// 		                                                static_cast<double>(fileSize)
	// 		                                         << "%" << std::endl;
	// 	                               },
	// 	                               [&](size_t fileSize, size_t) -> void {
	// 		                               std::lock_guard<std::mutex> lock(mut);
	// 		                               std::cout << "complete." << std::endl;
	// 	                               });
	// } catch (const std::runtime_error &e) {
	// 	std::cout << e.what() << std::endl;
	// }

#ifdef _MSC_VER
	SetProcessDPIAware();
#endif
	frame_ = new Frame();
	return true;
}

#ifdef NDEBUG
IMPLEMENT_APP(App);
#else
IMPLEMENT_APP_CONSOLE(App)
#endif