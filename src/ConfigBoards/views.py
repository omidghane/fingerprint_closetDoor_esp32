from django.shortcuts import render, get_object_or_404, redirect

from accounts.models import *
from django.urls import reverse
import requests
from django.contrib.auth.decorators import login_required

# Create your views here.

@login_required
def get_config_website(request):
    mac_id=request.GET.get("mac_id")

    device = DeviceFinger.objects.filter(mac_id=mac_id).first()

    # device = DeviceFinger.objects.filter(mac_id=mac_id)

    # url = 'http://' + device.static_ip + ':80/wificonfig'
    # respon = request.get(url)
    print(device.static_ip)

    context = {
        "device_ip" : device.static_ip ,
        "mac_id" : mac_id
    }

    return render(request, 'ConfigBoards/index.html', context)

def get_wificonfig(request):
    wifi_Mode = request.POST.get("Wifimode")
    SSID = request.POST.get("SSID")
    preshared = request.POST.get("presharedkey")
    APname = request.POST.get("APname")
    APsharedkey = request.POST.get("APsharedkey")
    mac_id = request.POST.get("mac_id")

    data = {
        'Wifimode': wifi_Mode, 'SSID': SSID, "presharedkey":preshared, "APname":APname, "APsharedkey":APsharedkey
    }
    device = get_object_or_404(DeviceFinger, mac_id=mac_id)
    url = 'http://' + device.static_ip + ':80/wificonfig'
    requests.get(url, params=data)


    base_url = reverse('ConfigBoards:config')  # 'target_view_name' is the name of the URL pattern
    query_string = f"?mac_id={mac_id}"

    # return Response({'detail': 'User Found'}, status=status.HTTP_200_OK)
    return redirect(base_url + query_string)
    
def get_networkconfig(request):
    DHCP = request.POST.get("DHCP")
    IP = request.POST.get("IP")
    SUBNET = request.POST.get("SUBNET")
    GATEWAY = request.POST.get("GATEWAY")
    DNS = request.POST.get("DNS")
    mac_id = request.POST.get("mac_id")
    data = {
        'DHCP': DHCP, 'IP': IP, "SUBNET":SUBNET, "GATEWAY":GATEWAY, "DNS":DNS
    }
    device = get_object_or_404(DeviceFinger, mac_id=mac_id)
    url = 'http://' + device.static_ip + ':80/networkconfig'
    requests.get(url, params=data)

    base_url = reverse('ConfigBoards:config')  # 'target_view_name' is the name of the URL pattern
    query_string = f"?mac_id={mac_id}"

    # return Response({'detail': 'User Found'}, status=status.HTTP_200_OK)
    return redirect(base_url + query_string)

def get_authconfig(request):
    USER = request.POST.get("USER")
    PASSWORD = request.POST.get("PASSWORD")
    mac_id = request.POST.get("mac_id")
    data = {
        'USER': USER, 'PASSWORD': PASSWORD
    }
    device = get_object_or_404(DeviceFinger, mac_id=mac_id)
    url = 'http://' + device.static_ip + ':80/authconfig'
    requests.get(url, params=data)

    base_url = reverse('ConfigBoards:config')  # 'target_view_name' is the name of the URL pattern
    query_string = f"?mac_id={mac_id}"

    # return Response({'detail': 'User Found'}, status=status.HTTP_200_OK)
    return redirect(base_url + query_string)

def get_command(request):
    ESP = request.POST.get("ESP")
    mac_id = request.POST.get("mac_id")
    data = {
        'ESP': ESP
    }
    device = get_object_or_404(DeviceFinger, mac_id=mac_id)
    url = 'http://' + device.static_ip + ':80/command'
    requests.get(url, params=data)

    base_url = reverse('ConfigBoards:config')  # 'target_view_name' is the name of the URL pattern
    query_string = f"?mac_id={mac_id}"

    # return Response({'detail': 'User Found'}, status=status.HTTP_200_OK)
    return redirect(base_url + query_string)