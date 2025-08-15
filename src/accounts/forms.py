from django import forms


class LoginForm(forms.Form):
    username = forms.EmailField(label='Username', max_length=100)
    password = forms.CharField(label='Password', max_length=100)


class RegisterForm(forms.Form):
    username = forms.EmailField(label='Username', max_length=100)
    password1 = forms.CharField(label='Password', max_length=100)
    password2 = forms.CharField(label='Confirm Password', max_length=100)
    is_personnel = forms.BooleanField(label='Is Personnel', required=False)

    def clean_password1(self):
        password1 = self.cleaned_data.get('password1')
        if len(password1) < 8:
            raise forms.ValidationError('رمز حداقل باید 8 کاراکتر باشد.')
        return password1

    def clean_password2(self):
        password1 = self.cleaned_data.get('password1')
        password2 = self.cleaned_data.get('password2')
        if password1 and password2 and password1 != password2:
            raise forms.ValidationError("رمزها یکسان نیستند.")
        if not any(s.isalpha() for s in password2):
            raise forms.ValidationError("حداقل یکی از کاراکترها باید از حروف باشند.")
        if not any(s.isupper() for s in password2):
            raise forms.ValidationError("حداقل یکی از کاراکترها باید از حروف بزرگ باشند.")
        if not any(s.isdigit() for s in password2):
            raise forms.ValidationError("حداقل یکی از کاراکترها باید عدد باشند.")
        return password2

class PhoneForm(forms.Form):
    phone = forms.CharField(label='Username', max_length=11)

    def clean_phone(self):
        phone = self.cleaned_data.get('phone')
        if phone[0] != '0':
            raise forms.ValidationError('شماره موبایل باید با 0 شروع شود.')
        if len(phone) != 11:
            raise forms.ValidationError('شماره موبایل شما باید 11 رقم باشد.')
        return phone


class CodeForm(forms.Form):
    code1 = forms.CharField(max_length=1)
    code2 = forms.CharField(max_length=1)
    code3 = forms.CharField(max_length=1)
    code4 = forms.CharField(max_length=1)
    code5 = forms.CharField(max_length=1)
    code6 = forms.CharField(max_length=1)
