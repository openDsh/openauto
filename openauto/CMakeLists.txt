add_library(openauto SHARED
        App.cpp
        Service/BluetoothService.cpp
        Service/InputService.cpp
        Service/MediaAudioService.cpp
        Service/ServiceFactory.cpp
        Service/AudioInputService.cpp
        Service/SystemAudioService.cpp
        Service/AndroidAutoEntityFactory.cpp
        Service/AudioService.cpp
        Service/SensorService.cpp
        Service/SpeechAudioService.cpp
        Service/Pinger.cpp
        Service/AndroidAutoEntity.cpp
        Service/VideoService.cpp
        Service/NavigationStatusService.cpp
        Service/MediaStatusService.cpp
        Configuration/RecentAddressesList.cpp
        Configuration/Configuration.cpp
        Projection/RemoteBluetoothDevice.cpp
        Projection/OMXVideoOutput.cpp
        Projection/LocalBluetoothDevice.cpp
        Projection/VideoOutput.cpp
        Projection/InputDevice.cpp
        Projection/SequentialBuffer.cpp
        Projection/DummyBluetoothDevice.cpp
        Projection/QtVideoOutput.cpp
        Projection/GSTVideoOutput.cpp 
        Projection/QtAudioInput.cpp
        Projection/RtAudioOutput.cpp
        Projection/QtAudioOutput.cpp
        ${CMAKE_SOURCE_DIR}/btservice/AndroidBluetoothServer.cpp
        ${CMAKE_SOURCE_DIR}/btservice/AndroidBluetoothServer.cpp
        ${CMAKE_SOURCE_DIR}/btservice/AndroidBluetoothService.cpp
        ${CMAKE_SOURCE_DIR}/btservice/btservice.cpp
        ${CMAKE_SOURCE_DIR}/include/btservice/AndroidBluetoothServer.hpp
        ${CMAKE_SOURCE_DIR}/include/btservice/AndroidBluetoothService.hpp
        ${CMAKE_SOURCE_DIR}/include/btservice/IAndroidBluetoothServer.hpp
        ${CMAKE_SOURCE_DIR}/include/btservice/IAndroidBluetoothService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/App.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Configuration/Configuration.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Configuration/RecentAddressesList.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Configuration/AudioOutputBackendType.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Configuration/IConfiguration.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Configuration/IRecentAddressesList.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Configuration/HandednessOfTrafficType.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Configuration/BluetootAdapterType.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/NavigationStatusService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/MediaStatusService.hpp
	${CMAKE_SOURCE_DIR}/include/openauto/Service/MediaAudioService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/SensorService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/IPinger.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/IAndroidAutoEntity.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/SystemAudioService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/IAndroidAutoEntityEventHandler.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/AudioInputService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/VideoService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/IAndroidAutoEntityFactory.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/AudioService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/IServiceFactory.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/BluetoothService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/AndroidAutoEntity.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/ServiceFactory.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/SpeechAudioService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/AndroidAutoEntityFactory.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/IService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/Pinger.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Service/InputService.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/IInputDeviceEventHandler.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/IVideoOutput.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/RtAudioOutput.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/LocalBluetoothDevice.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/IAudioOutput.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/QtAudioInput.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/VideoOutput.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/IBluetoothDevice.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/OMXVideoOutput.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/IAudioInput.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/DummyBluetoothDevice.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/IInputDevice.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/InputDevice.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/QtAudioOutput.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/RemoteBluetoothDevice.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/QtVideoOutput.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/SequentialBuffer.hpp
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/InputEvent.hpp
        )

if(GST_BUILD)
target_sources(openauto PRIVATE	
        ${CMAKE_SOURCE_DIR}/include/openauto/Projection/GSTVideoOutput.hpp
        )

target_include_directories(openauto SYSTEM PUBLIC
        ${QTGSTREAMER_INCLUDE_DIR}
        ${GST_INCLUDE_DIRS}
        )

target_link_libraries(openauto PRIVATE
        ${QTGSTREAMER_LIBRARY}
        ${GST_LIBRARIES}
        ${Qt5QuickWidgets_LIBRARIES}
        ${QTGSTREAMER_QUICK_LIBRARY}
        )
endif()

target_include_directories(openauto PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${RTAUDIO_INCLUDE_DIRS}
        ${Protobuf_INCLUDE_DIRS}
        )

target_link_libraries(openauto PUBLIC
        libusb
        Threads::Threads
        ${Boost_LIBRARIES}
        aasdk
        rtaudio
        Qt5::Bluetooth
        Qt5::MultimediaWidgets
        btservice_proto
        ${Protobuf_LIBRARIES}
        )

install(TARGETS openauto
        LIBRARY DESTINATION lib)

set_target_properties(openauto
        PROPERTIES INSTALL_RPATH_USE_LINK_PATH 1)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/openauto DESTINATION include)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/btservice DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/OpenautoLog.hpp DESTINATION include)
