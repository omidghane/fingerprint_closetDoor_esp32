from datetime import timezone, datetime, timedelta

import requests
from django.urls import reverse

from django.db.models import Max
from django.shortcuts import render, redirect, get_object_or_404
from django.contrib.auth import authenticate, login, logout, get_user_model
from django.contrib.auth.decorators import login_required
from django.contrib import messages
from rest_framework import viewsets, status
from rest_framework.decorators import action
from rest_framework.permissions import IsAuthenticated
from rest_framework.response import Response

from .forms import *
from .models import *
import random
from src.utils.sendsms import send_sms
from .serializers import *

from rest_framework_simplejwt.views import (
    TokenObtainPairView,
    TokenRefreshView,
)

User = get_user_model()


def login_view(request):
    if not request.user.is_authenticated:
        if request.method == "POST":
            form = LoginForm(request.POST)
            if form.is_valid():
                username = form.cleaned_data.get("username")
                password = form.cleaned_data.get("password")
                user = authenticate(
                    request, username=username, password=password
                )
                if user is not None:
                    login(request, user)
                    request.session.set_expiry(3600)
                    return redirect(request.GET.get("next", '/'))
                else:
                    messages.error(request, "نام کاربری یا رمز عبور اشتباه می باشد.", 'danger')
            else:
                return render(request, "accounts/login.html", {"form": form})

        form = LoginForm()
        context = {"form": form, "next": request.GET.get("next", '/')}
        return render(request, "accounts/login.html", context)
    else:
        next = request.GET.get("next", '/')
        return redirect(next)


@login_required
def logout_view(request):
    logout(request)
    return redirect("/")


def register_view(request):
    if request.method == "POST":
        form = RegisterForm(request.POST)
        if form.is_valid():
            username = form.cleaned_data.get("username")
            password = form.cleaned_data.get("password1")
            user = User.objects.filter(username=username)
            if user.exists():
                messages.error(request, "نام کاربری موجود می باشد.", 'danger')
                return redirect("accounts:register")
            else:
                user = User.objects.create_user(username=username, password=password,
                                                is_personnel=form.cleaned_data.get("is_personnel"))
                user.save()
                messages.success(request, "حساب کاربری با موفقیت ایجاد شد.", 'success')
                return redirect("accounts:login")
        else:
            return render(
                request, "accounts/register.html", {"form": form}
            )

    form = RegisterForm()
    context = {"form": form}
    return render(request, "accounts/register.html", context)


def verify(request, phone=None):
    if request.method == "POST":
        form = CodeForm(request.POST)
        if form.is_valid():
            code1 = form.cleaned_data.get("code1")
            code2 = form.cleaned_data.get("code2")
            code3 = form.cleaned_data.get("code3")
            code4 = form.cleaned_data.get("code4")
            code5 = form.cleaned_data.get("code5")
            code6 = form.cleaned_data.get("code6")

            code = str(code1) + str(code2) + str(code3) + str(code4) + str(code5) + str(code6)

            true_code = PhoneLogin.objects.filter(phone=phone, code=code, expires_at__gt=datetime.now(), used=False)

            if true_code.exists():
                true_code.update(used=True)
                return redirect('accounts:user_closet_form')
    else:
        form = CodeForm()
        context = {"phone": phone, "form": form}
        return render(request, "accounts/verify.html", context=context)


def login_phone_view(request):
    if request.method == "POST":
        form = PhoneForm(request.POST)
        if form.is_valid():
            phone = form.cleaned_data.get("phone")
            code = str(random.randint(100000, 999999))

            send_sms(phone, code)

            PhoneLogin.objects.create(phone=phone, code=code, created_at=datetime.now(),
                                      expires_at=(datetime.now() + timedelta(minutes=2)))

            return redirect('accounts:verify', phone=phone)

    form = PhoneForm()
    return render(request, "accounts/phone.html", context={'form': form})


class ProfileViewSet(viewsets.ModelViewSet):
    queryset = Profile.objects.all()
    serializer_class = ProfileSerializer
    # permission_classes = [IsAuthenticated]


class CustomTokenObtainPairView(TokenObtainPairView):
    serializer_class = CustomTokenObtainPairSerializer


class CustomTokenRefreshView(TokenRefreshView):
    pass


class UserFingerDoor(viewsets.ViewSet):
    serializer_class = UserDeviceSerializer
    permission_classes = [IsAuthenticated]

    def finger_insert(self, request):
        employee_code = request.data.get('employee_code')
        mac_id = request.data.get('mac_id')
        door_id = request.data.get('door_id')

        profile = get_object_or_404(Profile, employee_code=employee_code)
        user = get_object_or_404(User, pk=profile.user.pk)

        print(mac_id)
        device = get_object_or_404(DeviceFinger, mac_id=mac_id)
        # device = DeviceFinger.objects.filter(mac_id=mac_id).first()

        # user_device = UserDevice.objects.get_or_create(device=device, device_finger_id=door_id)

        data = {
            'finger_id': door_id, 'user_pk': user.pk
        }

        url = 'http://' + device.static_ip + ':80/enroll'
        # url = 'http://' + device.static_ip + ':60/enroll?finger_id=57&pk=70&user_pk=10043'


        requests.get(url, params=data)

        base_url = reverse('accounts:accounts-api:user_closet_form')  # 'target_view_name' is the name of the URL pattern
        query_string = f"?mac_id={mac_id}"

        # return Response({'detail': 'User Found'}, status=status.HTTP_200_OK)
        return redirect(base_url + query_string)

    def finger_insert_ack(self, request):
        door_id = request.data.get('door_id')
        mac_id = request.data.get('mac_id')
        user_pk = request.data.get('user_pk')
        finger_data = request.data.get('finger_data')

        user = get_object_or_404(User, pk=user_pk)
        device = get_object_or_404(DeviceFinger, mac_id=mac_id)

        user_device = UserDevice.objects.filter(device=device, device_finger_id=door_id)
        if user_device.exists():
            user_device.update(finger_data=finger_data, user=user)
        else:
            UserDevice.objects.create(device=device, device_finger_id=door_id, finger_data=finger_data, user=user)

        return Response(status=status.HTTP_201_CREATED)

    def delete_closet_door(self, request):
        employee_code = request.data.get('employee_code')
        mac_id = request.data.get('mac_id')
        door_id = request.data.get('door_id')

        profile = get_object_or_404(Profile, employee_code=employee_code)
        user = get_object_or_404(User ,pk = profile.user.pk)

    
        device = get_object_or_404(DeviceFinger, mac_id=mac_id)

        data = {
            'emp_id': door_id
        }

        url = 'http://' + device.static_ip + ':80/delete'


        response = requests.get(url, params=data)
        UserDevice.objects.filter(device=device.pk, user=user.pk, device_finger_id=door_id).delete()

        base_url = reverse('accounts:accounts-api:user_closet_form')  # 'target_view_name' is the name of the URL pattern
        query_string = f"?mac_id={mac_id}"

        # return Response({'detail': 'User Found'}, status=status.HTTP_200_OK)
        return redirect(base_url + query_string)

    def singledoor_unlock(self, request):
        finger_id = request.data.get('finger_id')
        mac_id = request.data.get('mac_id')
        action_time = request.data.get('action_time', datetime.now())

        device_id = get_object_or_404(DeviceFinger, mac_id=mac_id)

        user_device = get_object_or_404(UserDevice, device_finger_id=finger_id, device_id=device_id)

        device_log = DeviceLog.objects.create(user_device=user_device, action_time=action_time, is_open=False)

        door_ip = user_device.door_ip

        return Response({'door_ip': door_ip, 'device_log': device_log.pk}, status=status.HTTP_200_OK)

    def singledoor_unlock_ack(self, request):
        ack = request.data.get('ack')
        device_log_pk = request.data.get('device_log_pk')

        DeviceLog.objects.filter(pk=device_log_pk).update(is_open = ack)

        return Response(status=status.HTTP_200_OK)
    
    def closetdoor_unlock_ack(self, request):

        finger_id = request.data.get('finger_id')
        mac_id = request.data.get('mac_id')
        ack = request.data.get('ack')
        action_time = request.data.get('action_time', datetime.now())

        device = get_object_or_404(DeviceFinger, mac_id=mac_id)
        user_device = get_object_or_404(UserDevice, device=device, device_finger_id=finger_id)

        device_log = DeviceLog.objects.create(user_device=user_device, action_time=action_time, is_open=ack)
        # DeviceLog.objects.create()

        # device_log = get_object_or_404(DeviceLog, pk=device_log_pk)

        return Response(status=status.HTTP_200_OK)

    def log_insert(self, request):
        dev_mac_id = request.data.get('mac_id')
        finger_id = request.data.get('finger_id')
        action_time = request.data.get('action_time', datetime.now())

        device_pk = get_object_or_404(DeviceFinger, mac_id=dev_mac_id).pk
        user_device = UserDevice.objects.filter(device_finger_id=finger_id, device__id=device_pk)

        if user_device.exists():
            auth = user_device.first().authoritative
            
            device_log = DeviceLog.objects.create(user_device=user_device.first(), action_time=action_time)

            user = User.objects.get(pk=user_device.first().user.pk)
            profile = Profile.objects.get(user=user)

            return Response({'detail': 'Log Inserted Successfully', 'first_name': profile.first_name, 'last_name': profile.last_name, 'device_log_pk':device_log.pk, 'auth':auth}, status=status.HTTP_201_CREATED)
        else:
            return Response({'detail': 'Not Found'}, status=status.HTTP_404_NOT_FOUND)

@login_required
def user_finger_form(request):

    user = User.objects.filter(id=request.user.pk).first()
    device = DeviceFinger.objects.filter(mac_id=request.GET.get("mac_id")).first()
    userdevice = UserDevice.objects.filter(device_id=device.pk)
    # userdevice = get_object_or_404(UserDevice, device_id=device.pk)
    if not userdevice.exists():
        print("not exist")

    max_device_finger_id = userdevice.aggregate(Max('device_finger_id'))['device_finger_id__max']
    print(max_device_finger_id)

    return render(request, 'user_finger_forms.html', {"device":device, "user":user, "max_id":max_device_finger_id+1})

@login_required
def user_closet_form(request):
    
    user = User.objects.filter(id=request.user.pk).first()
    profile = get_object_or_404(Profile, user=user.pk)
    device = DeviceFinger.objects.filter(mac_id=request.GET.get("mac_id")).first()

    userdevice = UserDevice.objects.filter(device_id=device.pk ).exclude(user_id=1) 

    print(user.pk)
    myfingers = UserDevice.objects.filter(device_id=device.pk, user_id=user.pk)
    # print(myfingers)
    # if not userdevice.exists():
    #     print("notttttttt body")
    fingers = userdevice.values_list('device_finger_id', flat=True)
    myfingers = myfingers.values_list('device_finger_id', flat=True)
    closets= []
    for finger_id in range(1, 7):
        if finger_id in myfingers:
            closets.append("selected")
        elif finger_id in fingers:
            closets.append("unavailable")
        else:
            closets.append("empty")

    print(closets)


    # devices = DeviceFinger.objects.all()
    return render(request, 'locker_page.html', {"device":device, "profile":profile, "closets":list(closets)})


class PlantWateringClass(viewsets.ViewSet):
    serializer_class = UserDeviceSerializer
    permission_classes = [IsAuthenticated]

    def moisture_read(self, request):
        moisture = request.data.get('moisture')
        mac_id = request.data.get('mac_id')

        device = get_object_or_404(DeviceFinger, mac_id=mac_id)

        # Moisture.objects.filter(device=device).update_or_create(moisture=moisture, device=device)
        Moisture.objects.filter(device=device).create(moisture=moisture, device=device)
        return Response(status=status.HTTP_200_OK)


class TempratureClass(viewsets.ViewSet):
    serializer_class = UserDeviceSerializer
    permission_classes = [IsAuthenticated]

    def temprature_read(self, request):
        temprature = request.data.get('localTemp')
        humidity = request.data.get('localHum')
        mac_id = request.data.get('mac_id')
        action_time = request.data.get('action_time')

        device = get_object_or_404(DeviceFinger, mac_id=mac_id)

        Temprature.objects.create(device=device, humidity=humidity, temprature=temprature, action_time=action_time)

        return Response(status=status.HTTP_200_OK)

