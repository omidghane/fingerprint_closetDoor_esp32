from django.db import models
from django.contrib.auth.models import BaseUserManager, AbstractBaseUser, PermissionsMixin
from datetime import datetime


class UserManager(BaseUserManager):
    def create_user(self, username, password=None, **extra_fields):
        if not username:
            raise ValueError('Users must have username')

        extra_fields.setdefault('is_personnel', False)
        user = self.model(username=username, **extra_fields)
        user.set_password(password)
        user.save()
        return user

    def create_superuser(self, username, password, **extra_fields):
        extra_fields.setdefault('is_staff', True)
        extra_fields.setdefault('is_active', True)
        extra_fields.setdefault('is_superuser', True)

        if extra_fields.get('is_staff') is not True:
            raise ValueError('Superuser must have is_staff=True.')

        return self.create_user(username, password, **extra_fields)

class User(AbstractBaseUser, PermissionsMixin):
    username = models.EmailField(max_length=100, unique=True)

    is_staff = models.BooleanField(default=False)
    is_active = models.BooleanField(default=True)
    is_superuser = models.BooleanField(default=False)
    is_personnel = models.BooleanField(default=False)

    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    REQUIRED_FIELDS = []
    objects = UserManager()

    USERNAME_FIELD = 'username'


class PhoneLogin(models.Model):
    phone = models.CharField(max_length=11)
    code = models.CharField(max_length=6)

    created_at = models.DateTimeField()
    expires_at = models.DateTimeField()

    used = models.BooleanField(default=False)


class Profile(models.Model):
    user = models.OneToOneField(User, on_delete=models.CASCADE, related_name='profile')
    first_name = models.CharField(max_length=100, blank=True, null=True)
    last_name = models.CharField(max_length=100, blank=True, null=True)
    employee_code = models.IntegerField(null=True)

    image = models.ImageField(upload_to='profile-images', blank=True, null=True)
    phone_number = models.CharField(max_length=10)

    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    def __str__(self):
        return f"{self.first_name} {self.last_name} - {self.phone_number} - {self.user.username}"
    

class DeviceFinger(models.Model):
    mac_id = models.CharField(max_length=30)
    city_name = models.CharField(max_length=30)
    x = models.FloatField()
    y = models.FloatField()
    static_ip = models.CharField(max_length=30, blank=True, null=True)
    is_for_door = models.BooleanField(default=False, blank=True, null=True)
    description = models.CharField(max_length=100, blank=True, null=True)

    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)


class UserDevice(models.Model):
    user = models.ForeignKey(User, on_delete=models.SET(1), blank=True, null=True)
    device = models.ForeignKey(DeviceFinger, on_delete=models.SET_NULL, blank=True, null=True)

    device_finger_id = models.IntegerField()
    finger_data = models.TextField(blank=True, null=True)

    door_ip = models.CharField(max_length=30, blank=True, null=True)
    authoritative = models.BooleanField(blank=True, null=True, default=0)

    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)


class DeviceLog(models.Model):
    user_device = models.ForeignKey(UserDevice, on_delete=models.SET_NULL, blank=True, null=True)
    action_time = models.DateTimeField(default=datetime.now)

    is_open = models.BooleanField(default=False, blank=True, null=True)

    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

class Moisture(models.Model):
    device = models.ForeignKey(DeviceFinger, on_delete=models.SET_NULL, blank=True, null=True)
    moisture = models.IntegerField()

    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)


class Temprature(models.Model):
    device = models.ForeignKey(DeviceFinger, on_delete=models.SET_NULL, blank=True, null=True)
    humidity = models.FloatField()
    temprature = models.FloatField()

    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)
    action_time = models.DateTimeField(default=datetime.now)

