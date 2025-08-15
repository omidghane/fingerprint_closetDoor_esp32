from rest_framework import serializers
from rest_framework_simplejwt.serializers import TokenObtainPairSerializer

from .models import *


class ProfileSerializer(serializers.ModelSerializer):
    user = serializers.ReadOnlyField(source='user.id')
    username = serializers.EmailField(source='user.username')
    # username2 = serializers.SerializerMethodField(method_name='get_email')

    class Meta:
        model = Profile
        fields = '__all__'

    def get_fields(self):
        fields = super().get_fields()
        request = self.context.get('request', None)

        return fields

    @staticmethod
    def get_email(obj):
        return obj.user.username

# class FingerPrintSerializer(serializers.ModelSerializer):
#
#     class Meta:
#         model = FingerPrint
#         fields = '__all__'

class CustomTokenObtainPairSerializer(TokenObtainPairSerializer):
    @classmethod
    def get_token(cls, user):
        token = super().get_token(user)

        return token

    def validate(self, attrs):
        data = super().validate(attrs)

        user = self.user
        data["user"] = user.id
        data["username"] = user.username
        data["firstname"] = user.profile.first_name
        data["lastname"] = user.profile.last_name
        data["user_image"] = 'http://192.168.2.6/' + user.profile.image.url

        return data


class UserDeviceSerializer(serializers.Serializer):
    employee_code = serializers.IntegerField()
    mac_id = serializers.CharField()
    first_name = serializers.CharField()
    last_name = serializers.CharField()
    finger_id = serializers.IntegerField()

class DeviceLogSerializer(serializers.ModelSerializer):

    class Meta:
        model = DeviceLog
        fields = '__all__'
