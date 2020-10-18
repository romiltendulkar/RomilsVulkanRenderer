#include "VulkanApplication.h"

int main()
{
	VulkanApplication *myApp = new VulkanApplication;
	try 
	{
		myApp->Run();
	}
	catch(const std::exception & e) 
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}